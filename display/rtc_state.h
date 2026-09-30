#pragma once
#include "esp_attr.h"

// Liegt im RTC-Speicher: übersteht den Tiefschlaf, nicht aber einen Stromausfall.
// Prüfsumme über die zuletzt gezeichneten Inhalte; 0 erzwingt einen Bildaufbau.
RTC_DATA_ATTR static uint32_t rtc_last_hash = 0;

// Prüfsummen je Abschnitt, um im Log zu sehen, was einen Bildaufbau ausgelöst hat.
RTC_DATA_ATTR static uint32_t rtc_part_hash[16] = {0};

// Zuletzt gezeichnete Werte für Toleranzvergleiche (Leistung, Verbrauch, Luftfeuchten).
RTC_DATA_ATTR static bool rtc_drawn_valid = false;
RTC_DATA_ATTR static int32_t rtc_drawn[12] = {0};
