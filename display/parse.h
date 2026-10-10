#pragma once
// esphome/display (JSON, v1) -> Model
#include <ArduinoJson.h>
#include <algorithm>
#include <cstring>
#include "model.h"

namespace dash {

// Returns false for an unknown version; the model is left unchanged then.
inline bool parse_model(JsonObjectConst x, Model &m) {
  if ((x["v"] | 0) != 1) return false;
  auto ival = [](JsonVariantConst v, int na) -> int { return v.isNull() ? na : (int) lroundf(v.as<float>()); };
  auto fval = [](JsonVariantConst v) -> float { return v.isNull() ? NAN : v.as<float>(); };
  auto sval = [](JsonVariantConst v) -> std::string { return v.is<const char *>() ? v.as<const char *>() : ""; };
  auto slist = [&](JsonVariantConst v, std::vector<std::string> &out) {
    out.clear();
    for (JsonVariantConst e : v.as<JsonArrayConst>()) out.push_back(sval(e));
  };
  auto tri = [](JsonVariantConst v) -> int { return v.is<bool>() ? (v.as<bool>() ? 1 : 0) : -1; };

  m.power = ival(x["power"], NA_P);
  m.en_today = fval(x["energy"]["today"]);
  m.en_yesterday = fval(x["energy"]["yesterday"]);

  JsonObjectConst out = x["out"].as<JsonObjectConst>();
  m.has_out = !out.isNull();
  m.t_out = fval(out["t"]);
  m.t_hi = ival(out["hi"], NA_T);
  m.t_lo = ival(out["lo"], NA_T);
  m.h_out = ival(out["h"], NA_H);
  const int b = ival(out["bft"], -1);
  m.bft = b < 0 ? -1 : std::min(b, 12);
  const int g = ival(out["gbft"], -1);
  m.gbft = g < 0 ? -1 : std::min(g, 12);
  m.detail = sval(out["detail"]);
  m.icon = m.has_out ? ival(out["icon"], 0) : -1;
  auto popval = [&](JsonVariantConst v) -> int { const int p = ival(v, -1); return p < 0 ? -1 : std::min(p, 100); };
  m.pop = popval(out["pop"]);
  m.night = out["night"] | false;
  m.sunrise = sval(out["sun"][0]);
  m.sunset = sval(out["sun"][1]);

  m.fc.clear();
  for (JsonObjectConst d : x["fc"].as<JsonArrayConst>()) {
    if (m.fc.size() >= 6) break;
    const int dg = d["gbft"] | -1;
    m.fc.push_back(
        Day{sval(d["n"]), d["i"] | 0, d["hi"] | 0, d["lo"] | 0, dg < 0 ? -1 : std::min(dg, 12), popval(d["pop"])});
  }

  for (int r = 0; r < ROOM_COUNT; r++) {
    JsonArrayConst a = x["rooms"][ROOM_KEYS[r]].as<JsonArrayConst>();
    const bool ok = !a.isNull() && a.size() >= 2;
    m.rooms[r].t = ok ? ival(a[0], NA_T) : NA_T;
    m.rooms[r].h = ok ? ival(a[1], NA_H) : NA_H;
  }

  auto rooms_list = [&](JsonVariantConst arr, std::vector<int> &out) {
    out.clear();
    for (JsonVariantConst k : arr.as<JsonArrayConst>()) {
      const char *key = k | "";
      for (int r = 0; r < ROOM_COUNT; r++)
        if (strcmp(key, ROOM_KEYS[r]) == 0) out.push_back(r);
    }
  };
  rooms_list(x["vent"], m.vent);
  rooms_list(x["shut"], m.shut);

  m.ct_open = ival(x["contacts"]["open"], NA_H);
  slist(x["contacts"]["names"], m.ct_names);
  m.attention = sval(x["attention"]);

  m.away = x["mode"]["away"] | false;
  m.holiday = x["mode"]["holiday"] | false;
  m.since = sval(x["mode"]["since"]);
  m.alarm = sval(x["alarm"]["text"]);
  m.alarm_at = sval(x["alarm"]["at"]);
  m.scene = sval(x["scene"]["name"]);
  m.scene_since = sval(x["scene"]["since"]);
  slist(x["devices"]["batt"], m.batt);
  slist(x["devices"]["dead"], m.dead);

  JsonObjectConst car = x["car"].as<JsonObjectConst>();
  m.car_valid = !car.isNull();
  m.car_windows = tri(car["windows"]);
  m.car_lids = tri(car["lids"]);
  m.car_service = tri(car["service"]);
  m.car_range = ival(car["range"], -1);
  m.car_at = sval(car["at"]);

  m.roll_fe = ival(x["roller"]["fenster"], -1);
  m.roll_tu = ival(x["roller"]["tuer"], -1);

  JsonObjectConst pr = x["print"].as<JsonObjectConst>();
  m.pr_active = !pr.isNull();
  m.pr_progress = ival(pr["p"], 0);
  m.pr_left = ival(pr["left"], -1);
  m.pr_tool = fval(pr["tool"]);
  m.pr_bed = fval(pr["bed"]);

  m.valid = true;
  return true;
}

}  // namespace dash
