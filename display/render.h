#pragma once
// Rendering of the home display (800 x 480): layout A daily overview, layout B away.
// A template so the same code runs on the ESP32 and in the PC comparison test.
// Texts shown on the display are German on purpose.
#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>
#include "model.h"

namespace dash {

using esphome::Color;
using esphome::display::BaseFont;
using esphome::display::COLOR_OFF;
using esphome::display::COLOR_ON;
using esphome::display::TextAlign;

struct Fonts {
  BaseFont *label, *room, *text, *hint, *hint_b, *fc, *value, *head, *kw, *big, *tile, *icon, *icon_big, *icon_s, *icon_xs;
};

// ---------- Formatting ----------
inline std::string de1(float v) {  // one decimal, decimal comma
  char b[16];
  snprintf(b, sizeof(b), "%.1f", v);
  for (char *p = b; *p; p++)
    if (*p == '.') *p = ',';
  return std::string(b);
}
inline std::string deg(int v) { return v == NA_T ? std::string("–") : esphome::to_string(v) + "°"; }
inline std::string pct(int v) { return v == NA_H ? std::string("–") : esphome::to_string(v) + " %"; }
inline std::string join(const std::vector<std::string> &v, const char *sep) {
  std::string r;
  for (const auto &s : v) {
    if (!r.empty()) r += sep;
    r += s;
  }
  return r;
}
inline std::string t_out_text(const Model &m) {  // outdoor temperature (OWM, whole degrees from Node-RED)
  if (std::isnan(m.t_out)) return deg(NA_T);
  if (fabsf(m.t_out - roundf(m.t_out)) < 0.05f) return esphome::to_string((int) roundf(m.t_out)) + "°";
  return de1(m.t_out) + "°";
}
inline std::string power_text(const Model &m) {
  if (m.power == NA_P) return "–";
  if (m.power >= 1000) return de1(m.power / 1000.0f) + " kW";
  return esphome::to_string(m.power) + " W";
}
inline const char *owm_icon(int id, bool windy = false, bool night = false) {
  if (windy && id == 800) return "\U000F059D";                     // windy, clear sky
  if (windy && id > 800 && id <= 804) return "\U000F059E";         // windy, cloudy
  if (id >= 200 && id < 300) return "\U000F0593";                  // thunderstorm
  if (id == 511 || (id >= 611 && id <= 616)) return "\U000F067F";  // sleet
  if (id >= 502 && id < 600) return "\U000F0596";                  // heavy rain
  if (id >= 300 && id < 600) return "\U000F0597";                  // rain
  if (id >= 600 && id < 700) return "\U000F0598";                  // snow
  if (id >= 700 && id < 800) return "\U000F0591";                  // fog
  if (night && id == 800) return "\U000F0594";                    // clear night: moon
  if (night && (id == 801 || id == 802)) return "\U000F0F31";     // partly cloudy night
  if (id == 800) return "\U000F0599";                              // sunny
  if (id == 801 || id == 802) return "\U000F0595";                 // partly cloudy
  return "\U000F0590";                                             // cloudy
}
static const char *const ICON_UMBRELLA = "\U000F054A";  // rain probability
static const char *const ICON_SUNRISE = "\U000F059C";   // sunrise/sunset times in the "HEUTE" header
static const char *const ICON_SUNSET = "\U000F059B";
static const char *const ICON_WIND = "\U000F059D";      // wind force when "Wind 3" does not fit
inline std::string shutter_text(int pos) {
  if (pos < 0) return "–";
  if (pos >= 100) return "offen";
  if (pos <= 0) return "geschlossen";
  return esphome::to_string(pos) + " % offen";
}

// ---------- Drawing ----------
template<typename D> struct Painter {
  D &it;
  const Fonts &F;

  int width_of(const std::string &s, BaseFont *f) {
    int x1, y1, w, h;
    it.get_text_bounds(0, 0, s.c_str(), f, TextAlign::BASELINE_LEFT, &x1, &y1, &w, &h);
    return w;
  }
  std::string fit(const std::string &s, BaseFont *f, int max_w) {  // shorten with …
    if (width_of(s, f) <= max_w) return s;
    std::string t = s;
    while (!t.empty() && width_of(t + "…", f) > max_w) {
      t.pop_back();
      while (!t.empty() && (t.back() & 0xC0) == 0x80) t.pop_back();  // no partial UTF-8 characters
    }
    return t + "…";
  }
  void check_mark(int x, int y, Color c) {  // check mark, y = baseline
    for (int d = 0; d < 3; d++) {
      it.line(x, y - 8 + d, x + 5, y - 3 + d, c);
      it.line(x + 5, y - 3 + d, x + 15, y - 16 + d, c);
    }
  }
  void cross_mark(int x, int y, Color c) {
    for (int d = 0; d < 3; d++) {
      it.line(x + d, y - 15, x + 13 + d, y - 1, c);
      it.line(x + 13 + d, y - 15, x + d, y - 1, c);
    }
  }
};

// gray = false: everything black; gray = true: labels and lines in two gray levels
template<typename D, typename T>
void render(D &it, const Model &m, const Fonts &F, const T &now, bool gray = false) {
  Painter<D> P{it, F};
  const Color INK = COLOR_ON;
  const Color PAPER = COLOR_OFF;
  const Color DARK = gray ? Color(170, 170, 170, 170) : INK;  // names, units, outlines, hint bar
  const Color LIGHT = gray ? Color(85, 85, 85, 85) : INK;     // section labels, separators
  // label dark gray, value black; in black and white a single call as before
  auto label_value = [&](int x, int y, BaseFont *f, const std::string &label, const std::string &value) {
    if (!gray) {
      it.print(x, y, f, TextAlign::BASELINE_LEFT, (label + value).c_str());
      return;
    }
    it.print(x, y, f, DARK, TextAlign::BASELINE_LEFT, label.c_str());
    it.print(x + P.width_of(label, f), y, f, INK, TextAlign::BASELINE_LEFT, value.c_str());
  };
  const auto &R = m.rooms;
  const std::vector<std::string> &ct_names = m.ct_names;
  const bool ct_known = m.ct_open != NA_H;
  const bool ct_open = ct_known && m.ct_open > 0;

  // --- Header (both layouts) ---
  static const char *const WD[] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
  static const char *const MO[] = {"Januar", "Februar", "März",     "April",   "Mai",      "Juni",
                                   "Juli",   "August",  "September", "Oktober", "November", "Dezember"};
  const bool away = m.away || m.holiday;
  if (away) {
    std::string head = m.holiday ? "Urlaub" : "Abwesend";
    if (!m.since.empty()) head += " seit " + m.since;
    it.print(16, 34, F.head, TextAlign::BASELINE_LEFT, head.c_str());
  } else if (now.is_valid()) {
    it.printf(16, 34, F.head, TextAlign::BASELINE_LEFT, "%s, %d. %s", WD[now.day_of_week - 1], now.day_of_month,
              MO[now.month - 1]);
  }
  if (now.is_valid())
    it.printf(784, 34, F.room, LIGHT, TextAlign::BASELINE_RIGHT, "Stand %02d:%02d", now.hour, now.minute);
  it.filled_rectangle(16, 47, 768, 2, DARK);

  // ===================== Layout B: away =====================
  if (away) {
    // Status rows on the left: 0 = ok, 1 = problem (inverted), 2 = notice
    int y = 100;
    auto row = [&](int state, const std::string &text, const std::string &sub) {
      if (state == 1) {
        it.filled_rectangle(16, y - 34, 488, 48);
        P.cross_mark(30, y, PAPER);
        it.print(58, y, F.head, PAPER, TextAlign::BASELINE_LEFT, P.fit(text, F.head, 436).c_str());
      } else {
        if (state == 0)
          P.check_mark(28, y, DARK);
        else
          it.print(32, y, F.head, TextAlign::BASELINE_LEFT, "!");
        it.print(58, y, F.head, state == 0 ? DARK : INK, TextAlign::BASELINE_LEFT, P.fit(text, F.head, 446).c_str());
      }
      if (!sub.empty()) {
        y += 36;
        it.print(58, y, F.text, DARK, TextAlign::BASELINE_LEFT, P.fit(sub, F.text, 446).c_str());
      }
      y += 52;
    };

    if (m.batt_low) row(2, "Akku schwach", "");
    if (ct_known) {
      if (ct_open)
        row(1, esphome::to_string(m.ct_open) + (m.ct_open == 1 ? " Kontakt offen" : " Kontakte offen"),
            join(ct_names, " · "));
      else
        row(0, "Alle Fenster und Türen zu", "");
    }
    if (!m.alarm.empty())
      row(1, "Alarm " + m.alarm + " " + m.alarm_at, "");
    else
      row(0, "Kein Alarm", "");
    if (ct_known) {
      // two garage doors: in the garage and in the workshop
      bool ga_open = false, we_open = false;
      for (const auto &k : ct_names) {
        if (k == "GarageTor" || k == "GA-Tor") ga_open = true;
        if (k == "WerkstattTor" || k == "WE-Tor") we_open = true;
      }
      if (ga_open && we_open)
        row(1, "Beide Garagentore offen", "");
      else if (ga_open)
        row(1, "Garagentor Garage offen", "");
      else if (we_open)
        row(1, "Garagentor Werkstatt offen", "");
      else
        row(0, "Beide Garagentore zu", "");
    }
    if (m.car_valid && (m.car_windows >= 0 || m.car_lids >= 0)) {
      if (m.car_windows == 0)
        row(1, "Auto: Fenster offen", "");
      else if (m.car_lids == 0)
        row(1, "Auto: Türen oder Klappen offen", "");
      else
        row(0, "Auto: Fenster und Türen zu", "");
    }
    {
      const Room *wet = nullptr;
      for (const auto &r : R)
        if (r.h != NA_H && (wet == nullptr || r.h > wet->h)) wet = &r;
      if (wet != nullptr) {
        if (wet->h >= 70)
          row(2, std::string(wet->name) + " " + esphome::to_string(wet->h) + " % Feuchte", "");
        else
          row(0, "Feuchte höchstens " + esphome::to_string(wet->h) + " %", "");
      }
    }

    // Right column: house, car, devices
    it.line(520, 66, 520, 464, LIGHT);
    int ry = 84;
    auto label = [&](const char *t) {
      it.print(540, ry, F.label, LIGHT, TextAlign::BASELINE_LEFT, t);
      ry += 32;
    };
    auto kv = [&](const char *k, const std::string &v) {
      it.print(540, ry, F.room, DARK, TextAlign::BASELINE_LEFT, k);
      it.print(784, ry, F.hint_b, TextAlign::BASELINE_RIGHT, v.c_str());
      ry += 28;
    };
    label("HAUS");
    {
      const Room *lo = nullptr, *hi = nullptr;
      for (const auto &r : R) {
        if (r.t == NA_T) continue;
        if (lo == nullptr || r.t < lo->t) lo = &r;
        if (hi == nullptr || r.t > hi->t) hi = &r;
      }
      if (lo != nullptr) kv("Innen min", deg(lo->t) + " " + lo->abbr);
      if (hi != nullptr) kv("Innen max", deg(hi->t) + " " + hi->abbr);
    }
    kv("Leistung", power_text(m));
    if (!std::isnan(m.t_out)) kv("Außen", t_out_text(m));
    if (pop_shown(m.pop) >= 0) kv("Regen heute", pct(m.pop));
    ry += 18;
    if (m.car_valid) {
      label("AUTO");
      if (m.car_range >= 0) kv("Reichweite", esphome::to_string(m.car_range) + " km");
      if (m.car_service >= 0) kv("Service", m.car_service == 1 ? "fällig" : "nicht fällig");
      if (!m.car_at.empty()) kv("Stand", m.car_at);
      ry += 18;
    }
    label("GERÄTE");
    kv("Batterie leer", esphome::to_string(m.batt.size()));
    kv("Ohne Meldung", esphome::to_string(m.dead.size()));
    std::vector<std::string> dev = m.batt;
    dev.insert(dev.end(), m.dead.begin(), m.dead.end());
    for (size_t i = 0; i < dev.size() && i < 2 && ry < 470; i++) {
      it.print(540, ry, F.room, DARK, TextAlign::BASELINE_LEFT, P.fit(dev[i], F.room, 244).c_str());
      ry += 22;
    }
    return;
  }

  // ===================== Layout A: daily overview =====================
  // --- Today ---
  it.print(16, 74, F.label, LIGHT, TextAlign::BASELINE_LEFT, "HEUTE");
  if (!m.sunrise.empty() && !m.sunset.empty()) {  // sunrise and sunset, right-aligned up to the separator
    const int w_icon = P.width_of(ICON_SUNRISE, F.icon_xs), w_time = P.width_of(m.sunrise, F.label);
    const int w_set = P.width_of(m.sunset, F.label);
    const int x = 290 - (2 * (w_icon + 3) + w_time + w_set + 14);
    it.print(x, 74, F.icon_xs, LIGHT, TextAlign::BASELINE_LEFT, ICON_SUNRISE);
    it.print(x + w_icon + 3, 74, F.label, LIGHT, TextAlign::BASELINE_LEFT, m.sunrise.c_str());
    const int x2 = x + w_icon + 3 + w_time + 14;
    it.print(x2, 74, F.icon_xs, LIGHT, TextAlign::BASELINE_LEFT, ICON_SUNSET);
    it.print(x2 + w_icon + 3, 74, F.label, LIGHT, TextAlign::BASELINE_LEFT, m.sunset.c_str());
  }
  const std::string t_out = t_out_text(m);
  // Large temperature; the icon fills the space up to the separator (x = 296)
  const int t_w_big = P.width_of(t_out, F.big);
  const bool t_small = t_w_big > (m.icon >= 0 ? 226 : 276);
  BaseFont *f_out = t_small ? F.kw : F.big;
  it.print(12, 148, f_out, TextAlign::BASELINE_LEFT, t_out.c_str());
  if (m.icon >= 0) {
    const int free_l = 12 + P.width_of(t_out, f_out) + 10;  // left edge of the free space
    const int free_w = 292 - free_l;
    if (free_w >= 96)
      it.print(free_l + free_w / 2, 114, F.icon_big, DARK, TextAlign::CENTER, owm_icon(m.icon, m.windy, m.night));
    else
      it.print(270, 120, F.icon, DARK, TextAlign::CENTER, owm_icon(m.icon, m.windy, m.night));
  }
  {
    // high (bold) and low (light) as in the forecast, humidity, rain, wind. Wind as "Wind 3" if it fits,
    // otherwise as a small icon with the number, otherwise right in the description line
    int cx = 16;
    std::string s = deg(m.t_hi);
    it.print(cx, 178, F.hint_b, TextAlign::BASELINE_LEFT, s.c_str());
    cx += P.width_of(s, F.hint_b) + 7;
    s = deg(m.t_lo);
    it.print(cx, 178, F.text, LIGHT, TextAlign::BASELINE_LEFT, s.c_str());
    cx += P.width_of(s, F.text) + 14;
    s = pct(m.h_out);
    it.print(cx, 178, F.text, DARK, TextAlign::BASELINE_LEFT, s.c_str());
    cx += P.width_of(s, F.text) + 14;
    const int pop = pop_shown(m.pop);
    if (pop >= 0) {
      it.print(cx, 178, F.icon_s, TextAlign::BASELINE_LEFT, ICON_UMBRELLA);
      cx += P.width_of(ICON_UMBRELLA, F.icon_s) + 2;
      s = pct(pop);
      it.print(cx, 178, F.text, TextAlign::BASELINE_LEFT, s.c_str());
      cx += P.width_of(s, F.text) + 14;
    }
    int detail_w = 272;
    if (m.bft >= 0) {
      const std::string word = "Wind " + esphome::to_string(m.bft), num = esphome::to_string(m.bft);
      // the wind glyph fills its box almost to the right edge: 4 px gap so it looks like the umbrella's 2 px
      const int w_glyph = P.width_of(ICON_WIND, F.icon_s) + 4;
      const int w_icon = w_glyph + P.width_of(num, F.text);
      auto wind_icon = [&](int right, int y) {
        it.print(right - w_icon, y, F.icon_s, DARK, TextAlign::BASELINE_LEFT, ICON_WIND);
        it.print(right - w_icon + w_glyph, y, F.text, DARK, TextAlign::BASELINE_LEFT, num.c_str());
      };
      if (cx + P.width_of(word, F.text) <= 290) {
        it.print(290, 178, F.text, DARK, TextAlign::BASELINE_RIGHT, word.c_str());
      } else if (cx + w_icon <= 290) {
        wind_icon(290, 178);
      } else {
        wind_icon(290, 200);
        detail_w -= w_icon + 12;
      }
    }
    it.print(16, 200, F.text, DARK, TextAlign::BASELINE_LEFT, P.fit(m.detail, F.text, detail_w).c_str());
  }
  it.line(296, 62, 296, 200, LIGHT);

  // Context tiles; x = left edge of the column (middle 314, right 558)
  auto print_tile = [&](int x) {
    it.print(x, 74, F.label, LIGHT, TextAlign::BASELINE_LEFT, "3D-DRUCK");
    const int p = std::max(0, std::min(100, m.pr_progress));
    it.printf(x - 2, 122, F.tile, TextAlign::BASELINE_LEFT, "%d %%", p);
    if (gray) it.filled_rectangle(x, 134, 222, 16, LIGHT);  // remainder light gray
    it.rectangle(x, 134, 222, 16);
    it.rectangle(x + 1, 135, 220, 14);
    it.filled_rectangle(x, 134, 222 * p / 100, 16);
    if (m.pr_left >= 0)
      it.printf(x, 176, F.text, DARK, TextAlign::BASELINE_LEFT, "noch %d:%02d h", m.pr_left / 3600,
                (m.pr_left % 3600) / 60);
    if (!std::isnan(m.pr_tool) && !std::isnan(m.pr_bed))
      it.printf(x, 200, F.room, DARK, TextAlign::BASELINE_LEFT, "Düse %.0f° · Bett %.0f°", m.pr_tool, m.pr_bed);
  };
  auto scene_tile = [&](int x) {
    it.print(x, 74, F.label, LIGHT, TextAlign::BASELINE_LEFT, "SZENE AKTIV");
    it.print(x - 2, 122, F.tile, TextAlign::BASELINE_LEFT, P.fit(m.scene, F.tile, 220).c_str());
    if (!m.scene_since.empty())
      it.printf(x, 158, F.text, DARK, TextAlign::BASELINE_LEFT, "seit %s", m.scene_since.c_str());
    const Room *r = nullptr;
    if (m.scene == "Sauna")
      r = &R[WK];
    else if (m.scene == "Kamin")
      r = &R[WZ];
    if (r != nullptr)
      it.printf(x, 184, F.text, DARK, TextAlign::BASELINE_LEFT, "%s %s %s", r->abbr, deg(r->t).c_str(), pct(r->h).c_str());
  };

  // --- Middle column: running 3D print or active scene, otherwise power and energy ---
  // (the forecast stays visible; with print and scene at the same time the scene moves to the right column)
  const bool scene_right = m.pr_active && !m.scene.empty();
  if (m.pr_active) {
    print_tile(314);
  } else if (!m.scene.empty()) {
    scene_tile(314);
  } else {
    it.print(314, 74, F.label, LIGHT, TextAlign::BASELINE_LEFT, "LEISTUNG");
    if (m.power != NA_P) {
      std::string num, unit;
      if (m.power >= 1000) {
        num = de1(m.power / 1000.0f);
        unit = " kW";
      } else {
        num = esphome::to_string(m.power);
        unit = " W";
      }
      it.print(310, 140, F.kw, TextAlign::BASELINE_LEFT, num.c_str());
      it.print(310 + P.width_of(num, F.kw), 140, F.head, DARK, TextAlign::BASELINE_LEFT, unit.c_str());
    } else {
      it.print(310, 140, F.kw, TextAlign::BASELINE_LEFT, "–");
    }
    if (!std::isnan(m.en_today)) label_value(314, 178, F.text, "Heute ", de1(m.en_today) + " kWh");
    if (!std::isnan(m.en_yesterday)) label_value(314, 200, F.text, "Gestern ", de1(m.en_yesterday) + " kWh");
  }
  it.line(540, 62, 540, 200, LIGHT);

  // --- Right column: context tile, otherwise forecast ---
  // Priority: scene (only while a print is shown in the middle), maintenance
  if (scene_right) {
    scene_tile(558);
  } else if (!m.batt.empty() || !m.dead.empty()) {
    it.filled_rectangle(548, 56, 236, 26, DARK);
    it.print(558, 74, F.label, PAPER, TextAlign::BASELINE_LEFT, "WARTUNG");
    int wy = 106;
    if (!m.batt.empty()) {
      it.print(558, wy, F.hint_b, TextAlign::BASELINE_LEFT, "Batterie leer");
      it.print(558, wy + 22, F.room, DARK, TextAlign::BASELINE_LEFT, P.fit(join(m.batt, ", "), F.room, 222).c_str());
      wy += 54;
    }
    if (!m.dead.empty()) {
      it.print(558, wy, F.hint_b, TextAlign::BASELINE_LEFT, "Ohne Meldung");
      it.print(558, wy + 22, F.room, DARK, TextAlign::BASELINE_LEFT, P.fit(join(m.dead, ", "), F.room, 222).c_str());
    }
  } else {
    it.print(558, 74, F.label, LIGHT, TextAlign::BASELINE_LEFT, "VORHERSAGE");
    int shown = 0;
    for (size_t i = 0; i < m.fc.size() && shown < 3; i++) {
      const int cx = 596 + shown * 76;
      it.print(cx, 102, F.room, DARK, TextAlign::BASELINE_CENTER, m.fc[i].n.c_str());
      it.print(cx, 134, F.icon, DARK, TextAlign::CENTER, owm_icon(m.fc[i].i, m.fc[i].windy));
      // high and low side by side, below the rain probability
      const std::string hi = esphome::to_string(m.fc[i].hi) + "°";
      std::string lo = esphome::to_string(m.fc[i].lo) + "°";
      const int w_hi = P.width_of(hi, F.fc);
      int w_lo = P.width_of(lo, F.room);
      const int gap = w_hi + w_lo + 7 <= 72 ? 7 : 3;  // narrower for wide winter values like "-5° -11°"
      if (w_hi + gap + w_lo > 74) {                    // "-10° -14": without the second degree sign
        lo = esphome::to_string(m.fc[i].lo);
        w_lo = P.width_of(lo, F.room);
      }
      const int x0 = cx - (w_hi + gap + w_lo) / 2;
      it.print(x0, 178, F.fc, TextAlign::BASELINE_LEFT, hi.c_str());
      it.print(x0 + w_hi + gap, 178, F.room, LIGHT, TextAlign::BASELINE_LEFT, lo.c_str());
      const int pop = pop_shown(m.fc[i].pop);
      if (pop >= 0) {
        const std::string ps = esphome::to_string(pop) + "%";
        const int w_u = P.width_of(ICON_UMBRELLA, F.icon_s), w_p = P.width_of(ps, F.room);
        const int px = cx - (w_u + 2 + w_p) / 2;
        it.print(px, 200, F.icon_s, DARK, TextAlign::BASELINE_LEFT, ICON_UMBRELLA);
        it.print(px + w_u + 2, 200, F.room, DARK, TextAlign::BASELINE_LEFT, ps.c_str());
      }
      shown++;
    }
    if (shown == 0) it.print(558, 110, F.room, DARK, TextAlign::BASELINE_LEFT, "noch keine Daten");
  }

  // --- Hint bar: only when action is needed ---
  const std::string attention =
      !m.batt_low ? m.attention : (m.attention.empty() ? std::string("Akku schwach") : "Akku schwach · " + m.attention);
  if (ct_open || !attention.empty()) {
    it.filled_rectangle(16, 216, 768, 44, DARK);
    int right_edge = 770;
    if (!attention.empty()) {
      const std::string a = P.fit(attention, F.hint, ct_open ? 360 : 740);
      it.print(770, 245, F.hint, PAPER, TextAlign::BASELINE_RIGHT, a.c_str());
      right_edge = 770 - P.width_of(a, F.hint) - 24;
    }
    if (ct_open) {
      const std::string head = esphome::to_string(m.ct_open) + " offen";
      it.print(30, 245, F.hint_b, PAPER, TextAlign::BASELINE_LEFT, head.c_str());
      const int nx = 30 + P.width_of(head, F.hint_b) + 14;
      std::string line;
      for (size_t k = 0; k < ct_names.size(); k++) {
        const std::string cand = line.empty() ? ct_names[k] : line + " · " + ct_names[k];
        if (nx + P.width_of(cand + " +9", F.hint) > right_edge) {
          line += " +" + esphome::to_string(ct_names.size() - k);
          break;
        }
        line = cand;
      }
      it.print(nx, 245, F.hint, PAPER, TextAlign::BASELINE_LEFT, line.c_str());
    }
  } else if (ct_known) {
    it.print(16, 245, F.text, DARK, TextAlign::BASELINE_LEFT, "Alle Fenster und Türen zu");
  }

  // --- House cross-section by floor (z2m/0..3) ---
  it.line(16, 318, 400, 274, DARK);
  it.line(16, 319, 400, 275, DARK);
  it.line(400, 274, 784, 318, DARK);
  it.line(400, 275, 784, 319, DARK);
  it.rectangle(16, 318, 768, 150, DARK);
  it.rectangle(17, 319, 766, 148, DARK);
  // Inner lines light gray; in grayscale kept inside the double outline so it is not interrupted
  const int inset = gray ? 2 : 0;
  it.line(16 + inset, 368, 783 - inset, 368, LIGHT);
  it.line(16 + inset, 418, 783 - inset, 418, LIGHT);
  it.line(400, 318 + inset, 400, 368, LIGHT);
  it.line(272, 418, 272, 468 - 2 * inset, LIGHT);
  it.line(528, 418, 528, 468 - 2 * inset, LIGHT);

  // Room: [floor] name  temperature  humidity; returns the width, draws only with draw = true
  auto room = [&](int x, int y, const char *floor, const Room &r, bool draw) -> int {
    int cx = x;
    if (floor != nullptr) {
      if (draw) it.print(cx, y, F.label, LIGHT, TextAlign::BASELINE_LEFT, floor);
      cx += P.width_of(floor, F.label) + 6;
    }
    if (draw) it.print(cx, y, F.room, DARK, TextAlign::BASELINE_LEFT, r.name);
    cx += P.width_of(r.name, F.room) + 10;
    const std::string v = deg(r.t) + "  " + pct(r.h);
    if (draw) it.print(cx, y, F.value, TextAlign::BASELINE_LEFT, v.c_str());
    return cx + P.width_of(v, F.value) - x;
  };
  {
    const int w = room(0, 0, "3", R[DB], false);
    room(400 - w / 2, 310, "3", R[DB], true);
  }
  room(30, 350, "2", R[KZ], true);
  room(414, 350, nullptr, R[SZ], true);
  room(30, 400, "1", R[WZ], true);

  // Living room shutters: the curtain is lowered in proportion to the position
  auto shutter = [&](int x, int y, int w, int h, int pos, bool door) {
    it.filled_rectangle(x - 3, y - 4, w + 6, 4, DARK);  // shutter box
    it.rectangle(x, y, w, h, DARK);
    if (door) {
      it.filled_rectangle(x + w - 7, y + h / 2, 3, 6, DARK);  // handle
    } else {  // window cross light gray, in grayscale without covering the frame
      const int gap = gray ? 1 : 0;
      it.line(x + w / 2, y + gap, x + w / 2, y + h - 1 - gap, LIGHT);
      it.line(x + gap, y + h / 2, x + w - 1 - gap, y + h / 2, LIGHT);
    }
    if (pos < 0) return;
    const int closed = (100 - std::max(0, std::min(100, pos))) * (h - 2) / 100;
    if (closed <= 0) return;
    it.filled_rectangle(x + 1, y + 1, w - 2, closed, DARK);
    for (int ly = y + 4; ly < y + 1 + closed; ly += 4) it.line(x + 2, ly, x + w - 3, ly, PAPER);  // slats
  };
  shutter(330, 374, 22, 38, m.roll_tu, true);
  label_value(364, 400, F.room, "Tür ", shutter_text(m.roll_tu));
  shutter(530, 378, 34, 30, m.roll_fe, false);
  label_value(576, 400, F.room, "Fenster ", shutter_text(m.roll_fe));
  room(30, 450, "0", R[WK], true);
  room(286, 450, nullptr, R[GA], true);
  room(542, 450, nullptr, R[WE], true);
}

}  // namespace dash
