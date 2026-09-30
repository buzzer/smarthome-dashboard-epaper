#pragma once
// Änderungserkennung: Muss das Bild neu gezeichnet werden?
// Exakt verglichen wird alles, was sichtbar ist (ohne "Stand HH:MM"); Leistung, Verbrauch heute und
// Luftfeuchten laufen über Toleranzen gegenüber den zuletzt gezeichneten Werten (RTC-Speicher).
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <string>
#include "model.h"
#include "rtc_state.h"

namespace dash {

// Toleranzen (Einheiten wie im Modell; Verbrauch in 0,1 kWh)
constexpr int TOL_POWER_W = 200;       // unter 1 kW
constexpr int TOL_POWER_KW_W = 500;    // ab 1 kW
constexpr int TOL_ENERGY_10WH = 5;     // 0,5 kWh
constexpr int TOL_HUM_OUT = 5;         // OWM springt stündlich
constexpr int TOL_HUM_ROOM = 3;

struct ChangeResult {
  bool redraw = false;
  std::string changed;    // exakte Abschnitte, die sich geändert haben
  std::string tolerance;  // Toleranzwerte, die überschritten sind
};

inline uint32_t fnv1a(const std::string &t) {
  uint32_t h = 2166136261u;
  for (char c : t) {
    h ^= (uint8_t) c;
    h *= 16777619u;
  }
  return h;
}

// today: Datum als JJJJMMTT (0, wenn die Uhrzeit fehlt). Aktualisiert bei Bildaufbau den RTC-Zustand.
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
  section("Datum");
  addi(m.away); addi(m.holiday); add(m.since);
  section("Modus");
  if (!std::isnan(m.en_yesterday)) addi(lroundf(m.en_yesterday * 10));
  section("Verbrauch gestern");
  addi(std::isnan(m.t_out) ? NA_T : (int) lroundf(m.t_out * 10));
  addi(m.t_hi); addi(m.t_lo); addi(m.bft); addi(m.icon);
  add(m.detail);
  section("Wetter");
  for (const auto &d : m.fc) { add(d.n); addi(d.i); addi(d.hi); addi(d.lo); }
  section("Vorhersage");
  addi(m.ct_open);
  for (const auto &n : m.ct_names) add(n);
  add(m.attention);
  section("Kontakte");
  add(m.alarm); add(m.alarm_at); add(m.scene); add(m.scene_since);
  for (const auto &b : m.batt) add(b);
  add("#");
  for (const auto &d : m.dead) add(d);
  section("Status");
  addi(m.car_valid); addi(m.car_windows); addi(m.car_lids); addi(m.car_service); addi(m.car_range); add(m.car_at);
  section("Auto");
  addi(m.pr_active);
  if (m.pr_active) {
    addi(m.pr_progress / 5);
    addi(m.pr_left / 600);
    addi(std::isnan(m.pr_tool) ? -1 : (int) lroundf(m.pr_tool / 5));
    addi(std::isnan(m.pr_bed) ? -1 : (int) lroundf(m.pr_bed / 5));
  }
  section("Drucker");
  addi(m.roll_fe); addi(m.roll_tu);
  section("Rolllaeden");
  for (const auto &r : m.rooms) addi(r.t);
  section("Raumtemperaturen");
  const uint32_t hash = fnv1a(all);

  // Toleranzen
  const int NA = INT32_MIN;
  auto hum = [&](int v) -> int { return v == NA_H ? NA : v; };
  const int cur[10] = {m.power == NA_P ? NA : m.power,
                       std::isnan(m.en_today) ? NA : (int) lroundf(m.en_today * 10),
                       hum(m.h_out),
                       hum(m.rooms[DB].h), hum(m.rooms[KZ].h), hum(m.rooms[SZ].h), hum(m.rooms[WZ].h),
                       hum(m.rooms[WK].h), hum(m.rooms[GA].h), hum(m.rooms[WE].h)};
  static const char *const NAMES[10] = {"Leistung",   "Verbrauch",  "Feuchte aussen", "Feuchte DB", "Feuchte KZ",
                                        "Feuchte SZ", "Feuchte WZ", "Feuchte WK",     "Feuchte GA", "Feuchte WE"};
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
