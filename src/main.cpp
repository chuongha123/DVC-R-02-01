#include "RestService.h"
#include "MqttService.h"
#include "ConfigStore.h"
#include "WifiService.h"
#include "WebService.h"
#include "TimerService.h"
#include "RelayService.h"
#include <time.h>
#include "sntp.h"

unsigned long keyPressStart = 0;
bool keyPressed = false;

static uint8_t lastCheckedMinute = 255; // Track last checked minute to avoid duplicate execution

// Initialize NTP
void initNTP()
{
    configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov"); // GMT+7 (Vietnam)
    Serial.println("[NTP] NTP initialized");
}

// Check and execute schedules
void checkSchedule()
{
    time_t now = time(nullptr);
    if (now < 1000000000) // NTP chưa sync (unixtime < 2001)
    {
        return;
    }

    struct tm *timeinfo = localtime(&now);
    if (!timeinfo)
        return;

    int currentDay = timeinfo->tm_wday; // 0=Sun, 1=Mon, ..., 6=Sat
    int currentHour = timeinfo->tm_hour;
    int currentMin = timeinfo->tm_min;

    // Tránh chạy nhiều lần trong cùng 1 phút
    if (lastCheckedMinute == currentMin)
        return;
    lastCheckedMinute = currentMin;

    // Convert Sunday=0 to Monday=0 (bitmask: bit 0=Mon, 1=Tue, ..., 6=Sun)
    int dayBit = (currentDay == 0) ? 6 : (currentDay - 1);

    ScheduleItem *schedules = ConfigStore_GetSchedules();
    int *scheduleCount = ConfigStore_GetScheduleCount();

    for (int i = 0; i < *scheduleCount; i++)
    {
        if (!schedules[i].enabled)
            continue;

        // Check day
        if (!(schedules[i].daysOfWeek & (1 << dayBit)))
            continue;

        // Check time (chính xác đến phút)
        if (schedules[i].hour == currentHour && schedules[i].minute == currentMin)
        {
            RelayService_Set(schedules[i].relayIndex, schedules[i].action);
            Serial.printf("[SCHEDULE] Executed: %s - Relay %d -> %s\n",
                         schedules[i].name[0] ? schedules[i].name : "Unnamed",
                         schedules[i].relayIndex,
                         schedules[i].action ? "ON" : "OFF");
        }
    }
}

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

    // Initialize NTP for schedule
    initNTP();

    // Check schedule every minute
    TimerService::setInterval(60000, []()
                              { checkSchedule(); });

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
