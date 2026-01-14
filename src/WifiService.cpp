#include "WifiService.h"


// AP password dùng cho chế độ cấu hình (định nghĩa trong Config.h)
// #define AP_PASSWORD "12345678"

static void startApOnly()
{
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);

    // Dùng DeviceId (SerialNo nếu đã set, ngược lại ESP8266-XXXXXX)
    String apSsid = ConfigStore_GetDeviceId();
    WiFi.softAP(apSsid.c_str(), AP_PASSWORD);


    Serial.println(F("[WiFi] AP-ONLY mode"));
    Serial.print(F("  AP SSID: "));
    Serial.println(apSsid);
    Serial.print(F("  AP IP  : "));
    Serial.println(WiFi.softAPIP());
}


void startWifi()
{
    WiFi.persistent(false);

    bool haveConfig = g_wifiCfg.ssid.length() > 0;

    if (haveConfig)
    {
        Serial.printf("[WiFi] Using saved SSID: %s\n", g_wifiCfg.ssid.c_str());

        WiFi.mode(WIFI_STA);

        // Static IP nếu có
        if (g_wifiCfg.use_static &&
            g_wifiCfg.ip != (uint32_t)0 &&
            g_wifiCfg.gw != (uint32_t)0 &&
            g_wifiCfg.mask != (uint32_t)0)
        {
            IPAddress dns1 = g_wifiCfg.dns1;
            IPAddress dns2 = g_wifiCfg.dns2;

            // fallback nếu lỡ bị 0.0.0.0
            if (dns1 == (uint32_t)0)
                dns1 = IPAddress(8, 8, 8, 8);
            if (dns2 == (uint32_t)0)
                dns2 = IPAddress(8, 8, 4, 4);

            Serial.println(F("[WiFi] Applying static IP config"));
            Serial.print(F("  IP   : "));
            Serial.println(g_wifiCfg.ip);
            Serial.print(F("  GW   : "));
            Serial.println(g_wifiCfg.gw);
            Serial.print(F("  MASK : "));
            Serial.println(g_wifiCfg.mask);
            Serial.print(F("  DNS1 : "));
            Serial.println(dns1);
            Serial.print(F("  DNS2 : "));
            Serial.println(dns2);

            // ESP8266 core mới có overload 5 tham số, nếu core cũ thì bỏ dns2 đi
            WiFi.config(g_wifiCfg.ip, g_wifiCfg.gw, g_wifiCfg.mask, dns1, dns2);
        }

        WiFi.begin(
            g_wifiCfg.ssid.c_str(),
            g_wifiCfg.pass.length() ? g_wifiCfg.pass.c_str() : nullptr);

        Serial.print(F("[WiFi] Connecting"));
        unsigned long t0 = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000)
        {
            delay(300);
            Serial.print(".");
        }
        Serial.println();

        if (WiFi.status() == WL_CONNECTED)
        {
            // ✅ CONNECTED → STA-only (tắt AP)
            WiFi.softAPdisconnect(true);
            WiFi.mode(WIFI_STA);

            Serial.println(F("[WiFi] STA connected"));
            Serial.print(F("  IP   : "));
            Serial.println(WiFi.localIP());
            Serial.print(F("  RSSI : "));
            Serial.println(WiFi.RSSI());
            return;
        }

        Serial.println(F("[WiFi] STA connect FAILED, fallback to AP-only"));
    }
    else
    {
        Serial.println(F("[WiFi] No WiFi config, AP-only mode"));
    }

    // ❌ Không có config hoặc connect thất bại → AP-only config
    startApOnly();
}
