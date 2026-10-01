#pragma once
// SNTP starts before WiFi is up (packages/battery.yaml enables WiFi only after the voltage measurement),
// so its first request fails and lwIP retries only 15 s later, often switching to the next server. To get
// the time right after connecting, SNTP is restarted from wifi.on_connect.
// esp_sntp_restart() and esp_sntp_enabled() loop forever in this ESP-IDF version (esp_sntp_enabled()
// ends up calling itself) until the task watchdog resets the board, so stop and init are queued directly;
// both only post to the lwIP thread and keep their order.
#include <esp_sntp.h>

namespace dash {
inline void sntp_restart() {
  esp_sntp_stop();
  esp_sntp_init();
}
}  // namespace dash
