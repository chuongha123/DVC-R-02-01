#pragma once

#include <EEPROM.h>
#include "ArduinoJson.h"

#define FIRMWARE_VERSION "3.0.3"
#define TOLERANCE_VALUE 35

// Tổng số byte EEPROM bạn đang dùng
#ifndef EEPROM_BYTES
#define EEPROM_BYTES 512
#endif

#define PIN_KEY 5

// Server (port + token hardcode)
#define SERVER_PORT 8080
#define SERVER_TOKEN "20fdjhsdfdgajkhbdsjbb5ae08ad6a7b0db4a"

// MQTT topic
#define API_PORT 1001
#define MQTT_PORT 1883
#define MQTT_USER "ViotBroker"
#define MQTT_PASS "Viot123!"
#define MQTT_PUB_INFO_TOPIC "device/info"
#define MQTT_SUB_CMD_TOPIC "device/cmd"

// Slot chia vùng trong EEPROM
// 0..255  : WifiConfig
// 256..511: ServerConfig + IrConfig + SerialNo
#define SLOT_WIFI_BASE 0
#define SLOT_SERVER_BASE 256

// GPIO pins cho giao tiếp P1P2
#define P1P2_RX_PIN GPIO_NUM_18
#define P1P2_TX_PIN GPIO_NUM_19  

// Relay pins (GPIO numbers) - Mảng để dễ dàng mở rộng
// Chỉ cần thêm pin vào mảng này để thêm relay mới
const int RELAY_PINS[] = {15, 2};
const int NUM_RELAYS = sizeof(RELAY_PINS) / sizeof(RELAY_PINS[0]);

// AP fallback password
#define AP_PASSWORD "12345678"

const String VALID_TOKEN = "DVC2023@IR";

struct WifiConfig
{
    String ssid;
    String pass;
    bool use_static = false;
    IPAddress ip;
    IPAddress mask;
    IPAddress gw;
    IPAddress dns1; // DNS1
    IPAddress dns2; // DNS2
};

struct ServerConfig
{
    String server_ip;            // IP / URL server
    uint16_t server_port = 1001; // nếu cần override SERVER_PORT
    String token;                // token tự lưu (có thể bỏ trống, vì token chính đang hardcode)
    String serial_no;            // SerialNo / DeviceId (ví dụ IR000111)
};

extern WifiConfig g_wifiCfg;
extern ServerConfig g_serverCfg;

// Set serial number & lưu (dùng cho /api/setSerialNo?SN=IR000111)
void ConfigStore_SetSerialNo(const String &sn);

// Xoá toàn bộ cấu hình (factory reset: WiFi + Server + IR + SerialNo)
void ConfigStore_FactoryResetAll();

String ConfigStore_GetDeviceId();

void ConfigStore_SaveServer(); // Lưu g_serverCfg + g_irCfg vào EEPROM (slot SERVER)

void ConfigStore_SaveWifi();

void ConfigStore_Init();