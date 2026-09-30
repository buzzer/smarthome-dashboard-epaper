// Test der Änderungserkennung (display/change.h): Folge von Aufwachvorgängen mit erwarteten Ergebnissen
#include "esphome_mock.h"
#include <iostream>
#include "../display/change.h"

int main() {
  int fails = 0;
  auto step = [&](const char *what, const dash::Model &m, bool expect) {
    const auto r = dash::changes(m, 20260930);
    const bool ok = r.redraw == expect;
    if (!ok) fails++;
    std::cout << (ok ? "OK    " : "FEHLER") << " " << what << " -> " << (r.redraw ? "zeichnen" : "unverändert")
              << (r.tolerance.empty() ? "" : "  [" + r.tolerance + "]") << std::endl;
  };
  dash::Model m;
  m.valid = true;
  m.power = 300; m.en_today = 2.0f; m.h_out = 60; m.t_out = 15;
  for (auto &r : m.rooms) { r.t = 21; r.h = 65; }
  m.ct_open = 1; m.ct_names = {"SZ-Fen"};

  step("erster Zyklus nach Start", m, true);
  step("nichts geändert", m, false);
  m.power = 490; step("Leistung +190 W (Toleranz 200)", m, false);
  m.power = 510; step("Leistung +210 W", m, true);
  m.power = 1400; step("Leistung auf 1,4 kW", m, true);
  m.power = 1800; step("Leistung +400 W über 1 kW (Toleranz 500)", m, false);
  m.h_out = 64; step("Feuchte außen +4 % (Toleranz 5)", m, false);
  m.h_out = 65; step("Feuchte außen +5 %", m, true);
  m.rooms[dash::WZ].h = 67; step("Feuchte Wohnzimmer +2 % (Toleranz 3)", m, false);
  m.rooms[dash::WZ].h = 68; step("Feuchte Wohnzimmer +3 %", m, true);
  m.en_today = 2.4f; step("Verbrauch +0,4 kWh (Toleranz 0,5)", m, false);
  m.en_today = 2.5f; step("Verbrauch +0,5 kWh", m, true);
  m.rooms[dash::KZ].t = 22; step("Raumtemperatur +1 °C", m, true);
  m.ct_open = 0; m.ct_names.clear(); step("Kontakt geschlossen", m, true);
  m.rooms[dash::GA].h = dash::NA_H; step("Raumwert fällt weg", m, true);
  step("wieder nichts geändert", m, false);
  std::cout << (fails ? "FEHLGESCHLAGEN" : "alle Fälle bestanden") << std::endl;
  return fails ? 1 : 0;
}
