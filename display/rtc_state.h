#pragma once
#include "esp_attr.h"

// Lives in RTC memory: survives deep sleep, but not a power loss.
// Checksum of the last drawn content; 0 forces a redraw.
RTC_DATA_ATTR static uint32_t rtc_last_hash = 0;

// Checksums per section, to see in the log what triggered a redraw.
RTC_DATA_ATTR static uint32_t rtc_part_hash[16] = {0};

// Last drawn values for tolerance checks (power, energy, humidities).
RTC_DATA_ATTR static bool rtc_drawn_valid = false;
RTC_DATA_ATTR static int32_t rtc_drawn[12] = {0};
