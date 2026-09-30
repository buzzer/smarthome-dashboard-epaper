# E-paper home dashboard 7.5″

ESPHome firmware for a Waveshare 7.5″ e-paper display (V2, 800 × 480) on the Waveshare ESP32 driver board.
The display wakes up from deep sleep on a schedule, receives one JSON document from Node-RED via MQTT
(`esphome/display`) and redraws only when something relevant changed, using 4 gray levels.

- `HomeDashboard1.yaml`: main file with the settings (substitutions)
- `packages/`: board and display, network/MQTT, fonts, wake and sleep cycle
- `display/`: data model, JSON parser, rendering (layout A daily overview, layout B away), change detection
- `components/epaper_gray/`: driver extension for 4 gray levels (GPL-3.0, see `NOTICE.md` there)
- `test/`: PC tests for rendering and change detection, using a mock of the ESPHome display API

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
