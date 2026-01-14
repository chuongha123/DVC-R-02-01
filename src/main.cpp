#include "RestService.h"
#include "MqttService.h"
#include "ConfigStore.h"
#include "WifiService.h"
#include "WebService.h"
#include "TimerService.h"
#include "RelayService.h"

unsigned long keyPressStart = 0;
bool keyPressed = false;

void handleKey()
{
    bool currentState = !digitalRead(PIN_KEY); // Đảo ngược vì dùng PULLUP

    if (currentState && !keyPressed)
    {
        keyPressStart = millis();
        keyPressed = true;
    }
    else if (!currentState && keyPressed)
    {
        if (millis() - keyPressStart >= 5000)
        {
            Serial.println("Factory reset triggered!");
            ConfigStore_FactoryResetAll();
            delay(500);
            ESP.restart();
        }
        keyPressed = false;
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    pinMode(PIN_KEY, INPUT_PULLUP);

    // Relay init (GPIO15 & GPIO2)
    RelayService_Begin();

    // AC support removed; no P1P2 initialization

    ConfigStore_Init(); // bên trong làm EEPROM.begin(512) hoặc dung lượng bạn chọn

    // WiFi:
    // - nếu có config → thử STA
    // - nếu thất bại / chưa có SSID → AP-only với SSID = SerialNo (IR000xxx)
    startWifi();

    // Web server (UI 4 tab, API /wifi/save, /ir/scan, /ir/save, /ir/control,
    // /server/save, /api/getIP, /api/setSerialNo, /system/update, /system/restart, /system/factory_reset, ...)
    WebService_Begin();

    MqttService::init();
    String SerialNo = ConfigStore_GetDeviceId();
    RestService::postNewGateway(SerialNo, SERVER_TOKEN);

    RestService::postDeviceSupportIR(SerialNo, "true", SERVER_TOKEN);

    // Gửi alive + unixtime ngay khi khởi động
    RestService::postUnixtimeAndAlive(SERVER_TOKEN);
    // Và mỗi 5 phút gửi lại
    TimerService::setInterval(300000, []()
                              { RestService::postUnixtimeAndAlive(SERVER_TOKEN); });

    // restAPIBegin();
    // mqttInit();
}

void loop()
{
    TimerService::tick();

    // factory reset
    handleKey();

    // HTTP server + OTA + các endpoint
    WebService_Handle();

    // MQTT client loop
    MqttService::loop();

    // Nhỏ thôi để đỡ quay vòng 100%
    delay(5);
}
