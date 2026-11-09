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
  m.wa_active = true; m.wa_progress = 10; m.wa_left = 3550; m.wa_end = "14:47"; step("washer starts (replaces power)", m, true);
  m.power = 700; m.en_today = 5.4f; step("power and energy change during the washer (not shown)", m, false);
  m.wa_left = 3500; m.wa_progress = 11; step("washer: 50 s less time left (same 10 minute step)", m, false);
  m.wa_left = 2900; m.wa_progress = 28; step("washer: 10 minutes later", m, true);
  m.wa_done = true; m.wa_left = 0; m.wa_end = "14:42"; step("washer finished", m, true);
  m.wa_active = false; step("washer off (power shown again)", m, true);
  m.night = true; step("sunset alone (moon appears with the next redraw)", m, false);
  m.sunrise = "07:42"; m.sunset = "18:51"; step("sun times appear", m, true);
  m.sunrise = "07:44"; m.sunset = "18:49"; step("next day: sun times change", m, true);
  m.rooms[dash::KZ].h = 74; m.vent = {dash::KZ}; step("ventilation advised for KZ (humidity 74 %)", m, true);
  m.rooms[dash::KZ].h = 76; step("KZ humidity 74 -> 76 % while advised (within tolerance)", m, false);
  m.vent = {dash::WK, dash::KZ}; step("WK added and sorted first", m, true);
  m.vent = {dash::WK}; step("KZ no longer advised", m, true);
  m.vent.clear(); step("ventilation over", m, true);
  m.shut = {dash::KZ}; step("open window can be closed (KZ, no longer drawn)", m, false);
  m.shut = {dash::KZ, dash::GA}; step("garage door added to shut list (not drawn)", m, false);
  m.shut.clear(); step("shut list empty again", m, false);
  // car data never causes a redraw
  m.car_valid = true; m.car_online = 1; m.car_range = 400; m.car_open_known = true; step("car data appears", m, false);
  m.car_open = {"Heckklappe"}; step("car door opens", m, false);
  m.car_online = 0; m.car_range = 380; m.car_place = "Hauptstr. 1, Norderstedt"; m.car_place_at = "08.11. 12:05"; step("car offline, range and place change", m, false);
  m.car_odo = 179100; m.car_odo_min = true; m.car_service = "1.000 km"; m.car_name = "Karl"; step("odometer and inspection", m, false);
  m.car2_valid = true; m.car2_odo = 61200; m.car2_service = "25 Tage"; m.car2_name = "Honda"; step("second car appears", m, false);
  m.rooms[dash::KZ].t = 23; step("house data change still draws (with the car data on screen)", m, true);
  step("nothing changed again", m, false);
  std::cout << (fails ? "FAILED" : "all cases passed") << std::endl;
  return fails ? 1 : 0;
}
