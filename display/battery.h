#pragma once
// Battery state from the voltage on GPIO33 (packages/battery.yaml) and the "battery empty" screen.
#include <cmath>
#include "model.h"
#include "render.h"
#include "rtc_state.h"

namespace dash {

// Below this the ESP32 cannot run, so GPIO33 is not measuring the supply (pin not wired, powered via
// USB without a battery): treat it as "no measurement" instead of "empty".
constexpr float BATT_PLAUSIBLE_V = 2.5f;

// Sets m.vbat, m.batt_low (with hysteresis, kept in RTC memory) and m.batt_empty.
inline void apply_battery(Model &m, float v, float low_v, float ok_v, float empty_v) {
  if (!std::isnan(v) && v < BATT_PLAUSIBLE_V) v = NAN;
  m.vbat = v;
  if (std::isnan(v)) {  // no valid reading: never empty, keep the low hint as it was
    m.batt_empty = false;
    return;
  }
  m.batt_low = rtc_batt_low ? v < ok_v : v < low_v;
  rtc_batt_low = m.batt_low;
  m.batt_empty = v < empty_v;
}

// Drawn once when the battery is empty; the device then sleeps without WiFi until it is charged.
template<typename D> void render_empty(D &it, const Fonts &F, float v) {
  it.print(400, 230, F.tile, TextAlign::BASELINE_CENTER, "Akku leer");  // F.big only has digits
  if (!std::isnan(v)) {
    char b[16];
    snprintf(b, sizeof(b), "%.2f", v);
    for (char *p = b; *p; p++)
      if (*p == '.') *p = ',';
    it.printf(400, 280, F.head, TextAlign::BASELINE_CENTER, "Bitte laden · %s V", b);
  } else {
    it.print(400, 280, F.head, TextAlign::BASELINE_CENTER, "Bitte laden");
  }
}

}  // namespace dash
