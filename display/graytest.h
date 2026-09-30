#pragma once
// Testbild für den Graustufen-Betrieb (nur auf dem Gerät; zeigt, ob das Panel 4 Stufen kann)
#include "render.h"

namespace dash {

template<typename D> void render_graytest(D &it, const Fonts &F) {
  using esphome::epaper_gray::GRAY_DARK;
  using esphome::epaper_gray::GRAY_LIGHT;
  const Color INK = COLOR_ON;
  const Color PAPER = COLOR_OFF;

  it.print(16, 34, F.head, TextAlign::BASELINE_LEFT, "Graustufen-Test");
  it.filled_rectangle(16, 47, 768, 2);

  // vier Flächen
  struct Band { const char *name; Color c; };
  const Band bands[4] = {{"Weiß", PAPER}, {"Hellgrau", GRAY_LIGHT}, {"Dunkelgrau", GRAY_DARK}, {"Schwarz", INK}};
  for (int i = 0; i < 4; i++) {
    const int x = 16 + i * 192;
    it.filled_rectangle(x, 70, 184, 150, bands[i].c);
    it.rectangle(x, 70, 184, 150, INK);
    it.print(x + 92, 244, F.hint, TextAlign::BASELINE_CENTER, bands[i].name);
  }

  // Text in jeder Stufe
  it.print(16, 300, F.head, GRAY_LIGHT, TextAlign::BASELINE_LEFT, "Hellgrauer Text 21,5° 64 %");
  it.print(16, 340, F.head, GRAY_DARK, TextAlign::BASELINE_LEFT, "Dunkelgrauer Text 21,5° 64 %");
  it.print(16, 380, F.head, INK, TextAlign::BASELINE_LEFT, "Schwarzer Text 21,5° 64 %");

  // heller Text auf dunkler Fläche, Beschriftung wie im Layout
  it.filled_rectangle(16, 400, 768, 44, GRAY_DARK);
  it.print(30, 429, F.hint_b, PAPER, TextAlign::BASELINE_LEFT, "Weiß auf Dunkelgrau");
  it.print(770, 429, F.hint, GRAY_LIGHT, TextAlign::BASELINE_RIGHT, "Hellgrau auf Dunkelgrau");
  it.print(16, 470, F.label, GRAY_DARK, TextAlign::BASELINE_LEFT, "BESCHRIFTUNG DUNKELGRAU");
  it.print(400, 470, F.label, GRAY_LIGHT, TextAlign::BASELINE_LEFT, "BESCHRIFTUNG HELLGRAU");
}

}  // namespace dash
