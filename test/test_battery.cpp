// Test of the battery state (display/battery.h): hysteresis of the low hint, empty detection, rendering
#include "esphome_mock.h"
#include <iostream>
#include "../display/battery.h"

int main() {
  int fails = 0;
  auto check = [&](const char *what, bool got, bool expect) {
    const bool ok = got == expect;
    if (!ok) fails++;
    std::cout << (ok ? "OK    " : "FAIL  ") << " " << what << std::endl;
  };
  dash::Model m;
  auto step = [&](float v) { dash::apply_battery(m, v, 3.05f, 3.10f, 2.90f); };
  step(3.20f); check("3.20 V: not low", m.batt_low, false);
  step(3.07f); check("3.07 V: not low yet", m.batt_low, false);
  step(3.04f); check("3.04 V: low", m.batt_low, true);
  step(3.08f); check("3.08 V: stays low (hysteresis)", m.batt_low, true);
  step(3.11f); check("3.11 V: low off", m.batt_low, false);
  step(NAN);   check("no reading: keeps state", m.batt_low, false);
  step(2.95f); check("2.95 V: low, not empty", m.batt_low && !m.batt_empty, true);
  step(2.85f); check("2.85 V: empty", m.batt_empty, true);
  step(3.30f); check("3.30 V after charging: not empty", m.batt_empty, false);
  step(2.85f); step(0.30f); check("0.30 V (pin not on the supply): ignored, not empty", !m.batt_empty && std::isnan(m.vbat), true);

  static esphome::display::BaseFont FL{"label", 13}, FR{"room", 16}, FT{"text", 17}, FH{"hint", 18},
      FHB{"hint_b", 18}, FFC{"fc", 16}, FV{"value", 19}, FHD{"head", 24}, FK{"kw", 58}, FB{"big", 72},
      FTL{"tile", 40}, FI{"icon", 40}, FIB{"icon_big", 88}, FIS{"icon_s", 16};
  const dash::Fonts F{&FL, &FR, &FT, &FH, &FHB, &FFC, &FV, &FHD, &FK, &FB, &FTL, &FI, &FIB, &FIS};
  auto drawn = [&](const dash::Model &mm, const char *text) {
    esphome::display::Display d;
    dash::render(d, mm, F, MockTime{}, true);
    for (const auto &l : d.log)
      if (l.find(text) != std::string::npos) return true;
    return false;
  };
  dash::Model a;
  a.batt_low = true;
  check("layout A: hint bar shows it", drawn(a, "[Akku schwach]"), true);
  a.attention = "Batterie: Fenster";
  check("layout A: combined with other hint", drawn(a, "[Akku schwach · Batterie: Fenster]"), true);
  a.away = true;
  check("layout B: notice row", drawn(a, "[Akku schwach]"), true);
  dash::Model b;
  check("not low: nothing drawn", drawn(b, "Akku"), false);

  esphome::display::Display e;
  dash::render_empty(e, F, 2.853f);
  check("empty screen shows voltage", e.log.size() == 2 && e.log[1].find("[Bitte laden · 2,85 V]") != std::string::npos, true);

  std::cout << (fails ? "FAILED" : "all cases passed") << std::endl;
  return fails ? 1 : 0;
}
