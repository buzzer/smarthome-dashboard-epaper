#pragma once
// SNTP is initialised before WiFi is up (packages/battery.yaml starts WiFi only after the voltage
// measurement), so its first request goes nowhere and lwIP retries only after a timeout. Restarting it
// once the network is connected sends the first request right away.
#include <esp_sntp.h>

namespace dash {
inline void sntp_restart() {
  if (esp_sntp_enabled()) esp_sntp_restart();
}
}  // namespace dash
