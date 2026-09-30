# E-paper home dashboard 7.5″

ESPHome firmware for a Waveshare 7.5″ e-paper display (V2, 800 × 480) on the Waveshare ESP32 driver board.
The display wakes up from deep sleep on a schedule, receives one JSON document from Node-RED via MQTT
(`esphome/display`) and redraws only when something relevant changed, using 4 gray levels.

<img src="docs/layout-a-gray.svg" alt="Layout A: daily overview with 4 gray levels" width="800">

*Layout A, the daily overview: date, today's weather, power and energy, forecast, a hint bar for open
windows and doors, and a cross-section of the house with temperature and humidity per room and the
living room shutters. Design mockup with example values.*

<img src="docs/layout-b-away.svg" alt="Layout B: checklist while away, 4 gray levels" width="800">

*Layout B, shown while away or on holiday: a checklist (open contacts, alarm, garage doors, car,
humidity) and a summary of house, car and devices. Problems stay black, everything that is fine
recedes into gray. Design mockup with example values.*

While a 3D print is running, a scene is active or battery devices need attention, a context tile
replaces the forecast:

<img src="docs/tile-print.svg" alt="Tile: 3D print" width="244"> <img src="docs/tile-scene.svg" alt="Tile: active scene" width="244"> <img src="docs/tile-maintenance.svg" alt="Tile: maintenance" width="244">

- `HomeDashboard1.yaml`: main file with the settings (substitutions)
- `packages/`: board, display driver, network/MQTT, fonts, wake and sleep cycle
- `display/`: data model, JSON parser, rendering (layout A daily overview, layout B away), change detection
- `components/epaper_gray/`: driver extension for 4 gray levels (GPL-3.0, see `NOTICE.md` there)
- `test/`: PC tests for rendering and change detection, using a mock of the ESPHome display API

## Display variants

`HomeDashboard1.yaml` selects the display package:

- `packages/display_gray.yaml` (default): 4 gray levels. Values in black, labels, units and lines in
  gray. Uses `components/epaper_gray/` (GPL-3.0). Setting the substitution `grayscale: "false"`
  draws black and white with the same driver.
- `packages/display_bw.yaml`: black and white with ESPHome's standard `waveshare_epaper` driver.
  Gray elements are drawn in black; no GPL-licensed file from this project is used.

<img src="docs/layout-a-bw.svg" alt="Layout A in black and white" width="400">

## Language

Code, comments and logs are in English. The texts shown on the display are German on purpose
(weekdays, labels such as "Heute", "Vorhersage", "Stand"); they live in `display/render.h`.
Room abbreviations (`DB`, `KZ`, `WZ`, …) follow the German room names and are explained in
`display/model.h`.

## Setup

1. Create `secrets.yaml` with `wifi_ssid`, `wifi_password` and `home_dashboard_ota_password`.
2. Fonts: `Fonts/Verdana.ttf` and `Fonts/VerdanaBold.ttf` are not included (Microsoft, redistribution
   not permitted). Copy them from your own Windows or macOS installation into `Fonts/`.
   `Fonts/materialdesignicons.ttf` is from Pictogrammers (Material Design Icons).
3. `esphome run HomeDashboard1.yaml`

After the first flash the device runs in deep sleep. For OTA updates, enable maintenance mode in
Node-RED (retained `esphome/maintenance` = `on`); the device then stays awake on its next wake-up.

## Tests

From `test/`, with ArduinoJson from the ESPHome build directory:

```
AJ=../.esphome/build/home-dashboard-1/managed_components/bblanchon__arduinojson/src
g++ -std=gnu++20 -I mock -I $AJ -o build/test_change test_change.cpp && ./build/test_change
python3 build_compare.py <git-rev> scenarios
g++ -std=gnu++20 -I mock -I $AJ -o build/compare build/compare.cpp && ./build/compare scenarios/*.json
```

`build_compare.py` compares the drawing calls of an older YAML-lambda revision with the current C++
code for every scenario.

## License

- Own code and documentation: MIT, see `LICENSE`.
- `components/epaper_gray/`: GPL-3.0, because its grayscale waveforms come from GxEPD2
  (see `components/epaper_gray/NOTICE.md`). Only needed for the 4-gray-level variant; with
  `packages/display_bw.yaml` all project files in use are MIT.
- `Fonts/materialdesignicons.ttf`: Apache-2.0 (Pictogrammers Free License,
  https://github.com/Templarian/MaterialDesign-Webfont).
- Firmware built with ESPHome includes ESPHome's C++ runtime, which is GPL-3.0. This applies to any
  ESPHome firmware and only matters when distributing built firmware images.
