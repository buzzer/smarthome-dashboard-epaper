# epaper_gray – Lizenz und Herkunft

Diese Komponente steht unter der GNU General Public License Version 3 (siehe `LICENSE`).

## Fremder Code

- **Graustufen-Kurven** (`LUT20_VCOM` … `LUT25_BD` in `epaper_gray.cpp`) und die Befehlsfolge in
  `display_gray_lut_()` stammen aus `GxEPD2_750_T7Y.cpp` der Bibliothek GxEPD2_4G von Jean-Marc Zingg,
  Fork https://github.com/nicoh88/GxEPD2_4G. Der Fork enthält keine eigene Lizenzdatei; die
  zugrunde liegende Bibliothek GxEPD2 (https://github.com/ZinggJM/GxEPD2) steht unter GPL-3.0.
  Laut Quellcode gehen die Kurven auf Beispielcode von Good Display (GDEY075T7) zurück.
  Geändert: `LUT23_WB`, letzte Phase Richtung Schwarz 2 statt 1 Frame (dunkleres Hellgrau).
- **Graustufen mit Werkskurve** (`display_gray_otp_()`) folgt dem Beispielcode von Waveshare
  (EPD_7in5_V2, 4-Gray).
- **Basisklasse** `WaveshareEPaper7P5InV2P` aus ESPHome (https://github.com/esphome/esphome);
  der C++-Teil von ESPHome steht unter GPL-3.0.
