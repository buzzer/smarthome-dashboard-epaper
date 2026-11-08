// Prints the draw calls of one scenario (JSON file) as text lines, for check_glyphs.py.
// g++ -std=gnu++20 -I mock -I $AJ -o build/dump_scenario dump_scenario.cpp && ./build/dump_scenario scenarios/29_cars_two.json
#include "esphome_mock.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "../display/parse.h"
#include "../display/render.h"

int main(int argc, char **argv) {
  if (argc < 2) return 2;
  std::ifstream in(argv[1]);
  std::stringstream ss;
  ss << in.rdbuf();
  JsonDocument doc;
  if (deserializeJson(doc, ss.str())) return 2;
  dash::Model m;
  if (!dash::parse_model(doc.as<JsonObjectConst>(), m)) return 3;  // unknown version: nothing to draw
  // names as in packages/fonts.yaml (id without "f_"), sizes as there
  static esphome::display::BaseFont FL{"label", 13}, FR{"room", 16}, FT{"text", 17}, FH{"hint", 18}, FHB{"hint_b", 18}, FFC{"fc", 16},
      FV{"value", 19}, FHD{"head", 24}, FK{"kw", 58}, FB{"big", 72}, FTL{"tile", 40}, FI{"icon", 40}, FIB{"icon_big", 88},
      FIS{"icon_s", 16}, FIXS{"icon_xs", 13}, FRS{"room_s", 14}, FVS{"value_s", 18}, FO{"odo", 20}, FIC{"icon_car", 48},
      FICS{"icon_car_s", 20};
  const dash::Fonts F{&FL, &FR, &FT, &FH, &FHB, &FFC, &FV, &FHD, &FK, &FB, &FTL, &FI, &FIB, &FIS, &FIXS, &FRS, &FVS, &FO, &FIC, &FICS};
  esphome::display::Display d;
  dash::render(d, m, F, MockTime{}, true);
  for (const auto &l : d.log) std::cout << l << "\n";
  return 0;
}
