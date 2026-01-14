#pragma once

#include "WiFi.h"
#include "Arduino.h"
#include "ConfigStore.h"

// Khởi động WiFi:
// - Nếu có cấu hình, thử STA (có thể static IP hoặc DHCP).
// - Nếu kết nối thất bại hoặc chưa có cấu hình -> AP-only để cấu hình.
void startWifi();