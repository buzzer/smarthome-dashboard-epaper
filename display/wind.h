#pragma once
// Wind icon with hysteresis. Gusts from Beaufort 7 (where the German weather service starts warning
// about wind gusts) switch the dry-weather icons to their windy variants; they switch back only when
// gusts drop below Beaufort 6, so a value hovering at the threshold does not flip the icon.
// The state per day survives deep sleep in RTC memory.
#include <cstdint>
#include <string>
#include "change.h"
#include "model.h"
#include "rtc_state.h"

namespace dash {

constexpr int WINDY_ON_BFT = 7;
constexpr int WINDY_OFF_BFT = 6;

inline bool windy_next(bool was, int gbft) {
  if (gbft < 0) return false;
  return gbft >= (was ? WINDY_OFF_BFT : WINDY_ON_BFT);
}

// Sets m.windy and fc[i].windy from the gust values. Days are identified by their weekday name,
// which is unique within the forecast window. Calling it again with the same model gives the same result.
inline void apply_wind(Model &m) {
  auto was = [](uint32_t key) {
    for (int i = 0; i < rtc_windy_n; i++)
      if (rtc_windy_key[i] == key) return (rtc_windy_on >> i) & 1u;
    return 0u;
  };
  uint32_t keys[RTC_WINDY_SLOTS];
  uint8_t on = 0;
  int n = 0;
  auto update = [&](const std::string &name, int gbft) {
    const uint32_t key = fnv1a("wind:" + name);
    const bool w = windy_next(was(key) != 0, gbft);
    if (n < RTC_WINDY_SLOTS) {
      keys[n] = key;
      if (w) on |= 1u << n;
      n++;
    }
    return w;
  };
  m.windy = update("today", m.gbft);
  for (auto &d : m.fc) d.windy = update(d.n, d.gbft);
  for (int i = 0; i < n; i++) rtc_windy_key[i] = keys[i];
  rtc_windy_on = on;
  rtc_windy_n = n;
}

}  // namespace dash
