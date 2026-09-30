#pragma once
// Datenmodell der Hausanzeige: alles, was das Display zeigt, in einer Struktur.
// Gefüllt aus esphome/display (Node-RED "Display-Modell", v1), gelesen von render() und changes().
#include <cmath>
#include <string>
#include <vector>

namespace dash {

constexpr int NA_T = -128;    // Temperatur fehlt
constexpr int NA_H = 255;     // Feuchte oder Anzahl fehlt
constexpr int NA_P = -32768;  // Leistung fehlt

struct Room {
  const char *name;  // im Hausquerschnitt
  const char *abbr;  // in Layout B und Kacheln
  int t = NA_T;
  int h = NA_H;
};
enum RoomIdx { DB, KZ, SZ, WZ, WK, GA, WE, ROOM_COUNT };
static const char *const ROOM_KEYS[ROOM_COUNT] = {"db", "kz", "sz", "wz", "wk", "ga", "we"};

struct Day {
  std::string n;  // Wochentag, z. B. "Mi"
  int i = 0;      // OWM-Wetter-ID
  int hi = 0;
  int lo = 0;
};

struct Model {
  bool valid = false;  // in diesem Wachzyklus empfangen

  int power = NA_P;  // W
  float en_today = NAN, en_yesterday = NAN;  // kWh

  bool has_out = false;
  float t_out = NAN;  // °C
  int t_hi = NA_T, t_lo = NA_T;
  int h_out = NA_H;
  int bft = -1;   // Beaufort, -1 unbekannt
  int icon = -1;  // OWM-Wetter-ID für "Heute", -1 keins
  std::string detail;
  std::vector<Day> fc;  // die nächsten Tage (gezeigt werden höchstens 3)

  Room rooms[ROOM_COUNT] = {
      {"Dachboden", "Dachb."}, {"Kinderz.", "Kinderz."}, {"Schlafz.", "Schlafz."}, {"Wohnzimmer", "Wohnz."},
      {"Waschk.", "Waschk."},  {"Garage", "Garage"},     {"Werkstatt", "Werkst."},
  };

  int ct_open = NA_H;                  // Anzahl offener Kontakte
  std::vector<std::string> ct_names;   // Kurznamen, z. B. "SZ-Fen"
  std::string attention;               // Kurztext für den Hinweisbalken

  bool away = false, holiday = false;
  std::string since, alarm, alarm_at, scene, scene_since;
  std::vector<std::string> batt, dead;

  bool car_valid = false;
  int car_windows = -1, car_lids = -1, car_service = -1, car_range = -1;  // -1 unbekannt
  std::string car_at;

  int roll_fe = -1, roll_tu = -1;  // Rollläden Wohnzimmer, % offen

  bool pr_active = false;
  int pr_progress = 0, pr_left = -1;
  float pr_tool = NAN, pr_bed = NAN;
};

// Das eine Modell der Firmware (RAM; nach jedem Tiefschlaf neu aus MQTT)
static Model M;

}  // namespace dash
