// Test of the wind icon (display/wind.h, owm_icon in display/render.h): hysteresis across wake-ups
#include "esphome_mock.h"
#include <cstring>
#include <iostream>
#include "../display/wind.h"
#include "../display/render.h"

int main() {
  int fails = 0;
  auto check = [&](const char *what, bool got, bool expect) {
    const bool ok = got == expect;
    if (!ok) fails++;
    std::cout << (ok ? "OK    " : "FAIL  ") << " " << what << " -> " << (got ? "windy" : "calm") << std::endl;
  };
  dash::Model m;
  m.fc = {dash::Day{"Do", 803, 12, 8, 5}, dash::Day{"Fr", 800, 14, 7, 7}};

  // today: rising and falling gusts
  const int seq[] = {5, 6, 7, 6, 7, 5, 6};
  const bool exp[] = {false, false, true, true, true, false, false};
  const char *names[] = {"gusts 5", "gusts 6 (below switch-on)", "gusts 7 (switch on)", "gusts 6 (stays on)",
                         "gusts 7", "gusts 5 (switch off)", "gusts 6 (stays off)"};
  for (int i = 0; i < 7; i++) {
    m.gbft = seq[i];
    dash::apply_wind(m);
    check(names[i], m.windy, exp[i]);
  }
  m.gbft = -1;
  dash::apply_wind(m);
  check("gusts unknown", m.windy, false);

  // forecast days keep their own state, keyed by weekday
  check("Do gusts 5", m.fc[0].windy, false);
  check("Fr gusts 7", m.fc[1].windy, true);
  m.fc = {dash::Day{"Fr", 800, 14, 7, 6}, dash::Day{"Sa", 803, 11, 6, 6}};  // one day later
  dash::apply_wind(m);
  check("Fr gusts 6 a day later (stays on)", m.fc[0].windy, true);
  check("Sa gusts 6 (new day, below switch-on)", m.fc[1].windy, false);
  dash::apply_wind(m);
  check("same model again (idempotent)", m.fc[0].windy, true);

  // icons: only dry weather gets the windy variant
  auto icon = [&](const char *what, int id, bool windy, const char *expect, bool night = false) {
    const bool ok = std::strcmp(dash::owm_icon(id, windy, night), expect) == 0;
    if (!ok) fails++;
    std::cout << (ok ? "OK    " : "FAIL  ") << " icon " << what << std::endl;
  };
  icon("800 windy = weather-windy", 800, true, "\U000F059D");
  icon("803 windy = weather-windy-variant", 803, true, "\U000F059E");
  icon("500 windy stays rain", 500, true, "\U000F0597");
  icon("741 windy stays fog", 741, true, "\U000F0591");
  icon("800 calm = sunny", 800, false, "\U000F0599");
  // night: moon instead of sun, everything else unchanged
  icon("800 night = moon", 800, false, "\U000F0594", true);
  icon("801 night = partly cloudy night", 801, false, "\U000F0F31", true);
  icon("802 night = partly cloudy night", 802, false, "\U000F0F31", true);
  icon("803 night stays cloudy", 803, false, "\U000F0590", true);
  icon("500 night stays rain", 500, false, "\U000F0597", true);
  icon("800 windy night stays windy", 800, true, "\U000F059D", true);

  std::cout << (fails ? "FAILED" : "all cases passed") << std::endl;
  return fails ? 1 : 0;
}
