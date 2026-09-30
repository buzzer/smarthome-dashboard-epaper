#pragma once
// Data model of the home display: everything the display shows, in one struct.
// Filled from esphome/display (Node-RED "Display-Modell", v1), read by render() and changes().
#include <cmath>
#include <string>
#include <vector>

namespace dash {

constexpr int NA_T = -128;    // temperature missing
constexpr int NA_H = 255;     // humidity or count missing
constexpr int NA_P = -32768;  // power missing

struct Room {
  const char *name;  // in the house cross-section
  const char *abbr;  // in layout B and tiles
  int t = NA_T;
  int h = NA_H;
};
// Rooms, abbreviated by their German names (also used as JSON keys and on the display):
// DB = attic (Dachboden), KZ = children's room (Kinderzimmer), SZ = bedroom (Schlafzimmer),
// WZ = living room (Wohnzimmer), WK = laundry room (Waschküche), GA = garage (Garage),
// WE = workshop (Werkstatt)
enum RoomIdx { DB, KZ, SZ, WZ, WK, GA, WE, ROOM_COUNT };
static const char *const ROOM_KEYS[ROOM_COUNT] = {"db", "kz", "sz", "wz", "wk", "ga", "we"};

struct Day {
  std::string n;  // weekday, e.g. "Mi"
  int i = 0;      // OWM weather ID
  int hi = 0;
  int lo = 0;
};

struct Model {
  bool valid = false;  // received in this wake cycle

  int power = NA_P;  // W
  float en_today = NAN, en_yesterday = NAN;  // kWh

  bool has_out = false;
  float t_out = NAN;  // °C
  int t_hi = NA_T, t_lo = NA_T;
  int h_out = NA_H;
  int bft = -1;   // Beaufort, -1 unknown
  int icon = -1;  // OWM weather ID for "Heute" (today), -1 none
  std::string detail;
  std::vector<Day> fc;  // the next days (at most 3 are shown)

  Room rooms[ROOM_COUNT] = {
      {"Dachboden", "Dachb."}, {"Kinderz.", "Kinderz."}, {"Schlafz.", "Schlafz."}, {"Wohnzimmer", "Wohnz."},
      {"Waschk.", "Waschk."},  {"Garage", "Garage"},     {"Werkstatt", "Werkst."},
  };

  int ct_open = NA_H;                  // number of open contacts
  std::vector<std::string> ct_names;   // short names, e.g. "SZ-Fen"
  std::string attention;               // short text for the hint bar

  bool away = false, holiday = false;
  std::string since, alarm, alarm_at, scene, scene_since;
  std::vector<std::string> batt, dead;

  bool car_valid = false;
  int car_windows = -1, car_lids = -1, car_service = -1, car_range = -1;  // -1 unknown
  std::string car_at;

  int roll_fe = -1, roll_tu = -1;  // living room shutters, % open

  bool pr_active = false;
  int pr_progress = 0, pr_left = -1;
  float pr_tool = NAN, pr_bed = NAN;
};

// The firmware's single model (RAM; refilled from MQTT after every deep sleep)
static Model M;

}  // namespace dash
