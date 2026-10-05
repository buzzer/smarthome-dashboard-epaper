#pragma once
// Change detection: does the image need to be redrawn?
// Everything visible is compared exactly (except the "Stand HH:MM" time stamp); power, today's energy and
// humidities use tolerances against the last drawn values (RTC memory).
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <string>
#include "model.h"
#include "rtc_state.h"

namespace dash {

// Tolerances (units as in the model; energy in 0.1 kWh)
constexpr int TOL_POWER_W = 200;       // below 1 kW
constexpr int TOL_POWER_KW_W = 500;    // from 1 kW
constexpr int TOL_ENERGY_10WH = 5;     // 0.5 kWh
constexpr int TOL_HUM_OUT = 5;         // OWM jumps hourly
constexpr int TOL_HUM_ROOM = 3;

struct ChangeResult {
  bool redraw = false;
  std::string changed;    // exactly compared sections that changed
  std::string tolerance;  // tolerance values that were exceeded
};

inline uint32_t fnv1a(const std::string &t) {
  uint32_t h = 2166136261u;
  for (char c : t) {
    h ^= (uint8_t) c;
    h *= 16777619u;
  }
  return h;
}

// today: date as YYYYMMDD (0 if time is unknown). Updates the RTC state when a redraw is due.
inline ChangeResult changes(const Model &m, int today) {
  ChangeResult res;
  std::string s, all;
  int part = 0;
  auto add = [&](const std::string &v) { s += v; s += '|'; };
  auto addi = [&](int v) { add(esphome::to_string(v)); };
  auto section = [&](const char *name) {
    const uint32_t h = fnv1a(s);
    if (part < 16 && h != rtc_part_hash[part]) {
      res.changed += std::string(name) + "=" + s.substr(0, 60) + " ";
      rtc_part_hash[part] = h;
    }
    all += s;
    s.clear();
    part++;
  };

  addi(today);
  section("date");
  addi(m.away); addi(m.holiday); add(m.since);
  section("mode");
  if (!std::isnan(m.en_yesterday)) addi(lroundf(m.en_yesterday * 10));
  section("energy yesterday");
  addi(std::isnan(m.t_out) ? NA_T : (int) lroundf(m.t_out * 10));
  addi(m.t_hi); addi(m.t_lo); addi(m.bft); addi(m.icon); addi(m.windy); addi(pop_shown(m.pop));
  add(m.detail);
  section("weather");
  for (const auto &d : m.fc) { add(d.n); addi(d.i); addi(d.hi); addi(d.lo); addi(d.windy); addi(pop_shown(d.pop)); }
  section("forecast");
  addi(m.ct_open);
  for (const auto &n : m.ct_names) add(n);
  add(m.attention);
  addi(m.batt_low);
  section("contacts");
  add(m.alarm); add(m.alarm_at); add(m.scene); add(m.scene_since);
  for (const auto &b : m.batt) add(b);
  add("#");
  for (const auto &d : m.dead) add(d);
  section("status");
  addi(m.car_valid); addi(m.car_windows); addi(m.car_lids); addi(m.car_service); addi(m.car_range); add(m.car_at);
  section("car");
  addi(m.pr_active);
  if (m.pr_active) {
    addi(m.pr_progress / 5);
    addi(m.pr_left / 600);
    addi(std::isnan(m.pr_tool) ? -1 : (int) lroundf(m.pr_tool / 5));
    addi(std::isnan(m.pr_bed) ? -1 : (int) lroundf(m.pr_bed / 5));
  }
  section("printer");
  addi(m.roll_fe); addi(m.roll_tu);
  section("shutters");
  for (const auto &r : m.rooms) addi(r.t);
  section("room temperatures");
  const uint32_t hash = fnv1a(all);

  // Tolerances
  const int NA = INT32_MIN;
  auto hum = [&](int v) -> int { return v == NA_H ? NA : v; };
  const bool power_shown = m.scene.empty() && !m.pr_active;  // scene or print replace power and energy
  const int cur[10] = {m.power == NA_P || !power_shown ? NA : m.power,
                       std::isnan(m.en_today) || !power_shown ? NA : (int) lroundf(m.en_today * 10),
                       hum(m.h_out),
                       hum(m.rooms[DB].h), hum(m.rooms[KZ].h), hum(m.rooms[SZ].h), hum(m.rooms[WZ].h),
                       hum(m.rooms[WK].h), hum(m.rooms[GA].h), hum(m.rooms[WE].h)};
  static const char *const NAMES[10] = {"power",       "energy",      "humidity out", "humidity DB", "humidity KZ",
                                        "humidity SZ", "humidity WZ", "humidity WK",  "humidity GA", "humidity WE"};
  for (int i = 0; i < 10; i++) {
    const int last = rtc_drawn[i];
    if (cur[i] == last) continue;
    if (cur[i] == NA || last == NA || !rtc_drawn_valid) {
      res.tolerance += std::string(NAMES[i]) + " ";
      continue;
    }
    int thr = TOL_HUM_ROOM;
    if (i == 0) thr = (cur[i] >= 1000 || last >= 1000) ? TOL_POWER_KW_W : TOL_POWER_W;
    if (i == 1) thr = TOL_ENERGY_10WH;
    if (i == 2) thr = TOL_HUM_OUT;
    if (abs(cur[i] - last) >= thr)
      res.tolerance += std::string(NAMES[i]) + " " + esphome::to_string(last) + "->" + esphome::to_string(cur[i]) + " ";
  }

  res.redraw = (hash != rtc_last_hash) || !res.tolerance.empty();
  if (hash == rtc_last_hash) res.changed.clear();
  if (res.redraw) {
    rtc_last_hash = hash;
    for (int i = 0; i < 10; i++) rtc_drawn[i] = cur[i];
    rtc_drawn_valid = true;
  }
  return res;
}

}  // namespace dash
