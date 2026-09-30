# Hausanzeige 7,5″ E-Paper

ESPHome-Firmware für ein Waveshare 7,5″ E-Paper (V2, 800 × 480) am Waveshare ESP32-Treiberboard.
Das Display wacht im Tiefschlaf-Takt auf, holt ein JSON aus Node-RED über MQTT (`esphome/display`)
und zeichnet nur bei relevanten Änderungen neu, in 4 Graustufen.

- `HomeDashboard1.yaml`: Hauptdatei mit den Einstellungen (substitutions)
- `packages/`: Board und Display, Netzwerk/MQTT, Schriften, Wach- und Schlafzyklus
- `display/`: Datenmodell, JSON-Parser, Zeichnen (Layout A Tagesübersicht, Layout B Abwesend), Änderungserkennung
- `components/epaper_gray/`: Treiber-Erweiterung für 4 Graustufen (GPL-3.0, siehe `NOTICE.md` dort)
- `test/`: PC-Tests für Zeichnen und Änderungserkennung mit einer Nachbildung der ESPHome-Display-API

## Einrichten

1. `secrets.yaml` anlegen mit `wifi_ssid`, `wifi_password` und `home_dashboard_ota_password`.
2. Schriften: `Fonts/Verdana.ttf` und `Fonts/VerdanaBold.ttf` sind nicht enthalten (Microsoft,
   Weitergabe nicht erlaubt). Aus einer eigenen Windows- oder macOS-Installation nach `Fonts/` kopieren.
   `Fonts/materialdesignicons.ttf` stammt von Pictogrammers (Material Design Icons).
3. `esphome run HomeDashboard1.yaml`

Nach dem ersten Flashen läuft das Gerät im Tiefschlaf. Für OTA-Updates in Node-RED den
Wartungsmodus einschalten (retained `esphome/maintenance` = `on`).
