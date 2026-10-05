// Test of change detection (display/change.h): a sequence of wake-ups with expected results
#include "esphome_mock.h"
#include <iostream>
#include "../display/change.h"

int main() {
  int fails = 0;
  auto step = [&](const char *what, const dash::Model &m, bool expect) {
    const auto r = dash::changes(m, 20260930);
    const bool ok = r.redraw == expect;
    if (!ok) fails++;
    std::cout << (ok ? "OK    " : "FAIL  ") << " " << what << " -> " << (r.redraw ? "redraw" : "unchanged")
              << (r.tolerance.empty() ? "" : "  [" + r.tolerance + "]") << std::endl;
  };
  dash::Model m;
  m.valid = true;
  m.power = 300; m.en_today = 2.0f; m.h_out = 60; m.t_out = 15;
  for (auto &r : m.rooms) { r.t = 21; r.h = 65; }
  m.ct_open = 1; m.ct_names = {"SZ-Fen"};

  step("first cycle after boot", m, true);
  step("nothing changed", m, false);
  m.power = 490; step("power +190 W (tolerance 200)", m, false);
  m.power = 510; step("power +210 W", m, true);
  m.power = 1400; step("power to 1.4 kW", m, true);
  m.power = 1800; step("power +400 W above 1 kW (tolerance 500)", m, false);
  m.h_out = 64; step("outdoor humidity +4 % (tolerance 5)", m, false);
  m.h_out = 65; step("outdoor humidity +5 %", m, true);
  m.rooms[dash::WZ].h = 67; step("living room humidity +2 % (tolerance 3)", m, false);
  m.rooms[dash::WZ].h = 68; step("living room humidity +3 %", m, true);
  m.en_today = 2.4f; step("energy +0.4 kWh (tolerance 0.5)", m, false);
  m.en_today = 2.5f; step("energy +0.5 kWh", m, true);
  m.rooms[dash::KZ].t = 22; step("room temperature +1 °C", m, true);
  m.ct_open = 0; m.ct_names.clear(); step("contact closed", m, true);
  m.rooms[dash::GA].h = dash::NA_H; step("room value disappears", m, true);
  m.pop = 20; step("rain probability 20 % (not shown)", m, false);
  m.pop = 0; step("rain probability 0 % (not shown)", m, false);
  m.pop = 40; step("rain probability 40 % (shown)", m, true);
  m.pop = 50; step("rain probability 40 -> 50 %", m, true);
  m.fc = {dash::Day{"Do", 500, 12, 8, 3, 10}}; step("forecast day added", m, true);
  m.fc[0].pop = 20; step("forecast rain 10 -> 20 % (not shown)", m, false);
  m.fc[0].pop = 70; step("forecast rain 20 -> 70 %", m, true);
  m.scene = "Sauna"; step("scene starts (replaces power)", m, true);
  m.power = 3000; m.en_today = 4.0f; step("power and energy change during the scene (not shown)", m, false);
  m.scene.clear(); step("scene ends (power shown again)", m, true);
  m.pr_active = true; m.pr_progress = 10; step("print starts (replaces power)", m, true);
  m.power = 400; m.en_today = 4.8f; step("power and energy change during the print (not shown)", m, false);
  m.pr_active = false; step("print ends (power shown again)", m, true);
  step("nothing changed again", m, false);
  std::cout << (fails ? "FAILED" : "all cases passed") << std::endl;
  return fails ? 1 : 0;
}
