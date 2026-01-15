#include "ConfigStore.h"

// ====== Global instances (đúng với header) ======
WifiConfig g_wifiCfg;
ServerConfig g_serverCfg;


// Dùng đúng macro trong header
static const int SLOT_WIFI = SLOT_WIFI_BASE;
static const int SLOT_SERVER = SLOT_SERVER_BASE;
static const int SLOT_SCHEDULE = SLOT_SCHEDULE_BASE;

// Global schedule storage
static ScheduleItem schedules[MAX_SCHEDULES];
static int scheduleCount = 0;

// Forward declarations
ScheduleItem* ConfigStore_GetSchedules() { return schedules; }
int* ConfigStore_GetScheduleCount() { return &scheduleCount; }

// ====== CRC32 & blob helpers ======
static uint32_t crc32_update(uint32_t crc, const uint8_t *data, size_t len)
{
    crc = ~crc;
    while (len--)
    {
        crc ^= *data++;
        for (uint8_t k = 0; k < 8; k++)
        {
            crc = (crc & 1) ? (crc >> 1) ^ 0xEDB88320UL : (crc >> 1);
        }
    }
    return ~crc;
}

static bool eepromWriteBlob(int base, const uint8_t *data, uint16_t len)
{
    if (base < 0 || base + 4 + 2 + len + 4 > EEPROM_BYTES)
        return false;

    uint32_t sig = 0xA5A5A5A5;
    uint32_t crc = crc32_update(0, data, len);
    int pos = base;

    EEPROM.put(pos, sig);
    pos += 4;
    EEPROM.put(pos, len);
    pos += 2;

    for (uint16_t i = 0; i < len; i++)
        EEPROM.write(pos + i, data[i]);
    pos += len;

    EEPROM.put(pos, crc);
    pos += 4;

    return EEPROM.commit();
}

static bool eepromReadBlob(int base, uint8_t *out, uint16_t maxLen, uint16_t &outLen)
{
    outLen = 0;
    if (base < 0 || base + 4 + 2 + 4 > EEPROM_BYTES)
        return false;

    int pos = base;
    uint32_t sig = 0;
    EEPROM.get(pos, sig);
    pos += 4;
    if (sig != 0xA5A5A5A5)
        return false;

    uint16_t len = 0;
    EEPROM.get(pos, len);
    pos += 2;

    if (len == 0 || len > maxLen)
        return false;

    for (uint16_t i = 0; i < len; i++)
        out[i] = EEPROM.read(pos + i);
    pos += len;

    uint32_t savedCrc = 0;
    EEPROM.get(pos, savedCrc);
    pos += 4;

    uint32_t crc = crc32_update(0, out, len);
    if (crc != savedCrc)
        return false;

    outLen = len;
    return true;
}

static void eepromWipeSlot(int base, int bytes)
{
    for (int i = 0; i < bytes; i++)
        EEPROM.write(base + i, 0xFF);
}

static void loadWifiFromEeprom()
{
    // Default
    g_wifiCfg.ssid = "";
    g_wifiCfg.pass = "";
    g_wifiCfg.use_static = false;
    g_wifiCfg.ip = (uint32_t)0;
    g_wifiCfg.mask = (uint32_t)0;
    g_wifiCfg.gw = (uint32_t)0;
    g_wifiCfg.dns1 = (uint32_t)0;
    g_wifiCfg.dns2 = (uint32_t)0;

    uint8_t buf[256];
    uint16_t n = 0;
    if (!eepromReadBlob(SLOT_WIFI, buf, sizeof(buf), n))
        return;

    JsonDocument doc;
    if (deserializeJson(doc, buf, n) != DeserializationError::Ok)
        return;

    g_wifiCfg.ssid = String(doc["ssid"] | "");
    g_wifiCfg.pass = String(doc["pass"] | "");
    g_wifiCfg.use_static = doc["use_static"] | false;

    String strIp = (const char *)(doc["ip"] | "");
    String strMask = (const char *)(doc["mask"] | "");
    String strGw = (const char *)(doc["gw"] | "");
    String strDns1 = (const char *)(doc["dns1"] | "");
    String strDns2 = (const char *)(doc["dns2"] | "");

    if (strIp.length())
        g_wifiCfg.ip.fromString(strIp);
    if (strMask.length())
        g_wifiCfg.mask.fromString(strMask);
    if (strGw.length())
        g_wifiCfg.gw.fromString(strGw);
    if (strDns1.length())
        g_wifiCfg.dns1.fromString(strDns1);
    if (strDns2.length())
        g_wifiCfg.dns2.fromString(strDns2);
}

static void loadServerFromEeprom()
{
    // Default
    g_serverCfg.server_ip = "";
    g_serverCfg.server_port = 1001;
    g_serverCfg.token = "";
    g_serverCfg.serial_no = "";

    uint8_t buf[256];
    uint16_t n = 0;
    if (!eepromReadBlob(SLOT_SERVER, buf, sizeof(buf), n))
        return;

    JsonDocument doc;
    if (deserializeJson(doc, buf, n) != DeserializationError::Ok)
        return;

    // Server
    g_serverCfg.server_ip = String(doc["server_ip"] | "");
    g_serverCfg.server_port = (uint16_t)(doc["server_port"] | 1001);
    g_serverCfg.token = String(doc["token"] | "");
    g_serverCfg.serial_no = String(doc["serial_no"] | "");
}

static void loadScheduleFromEeprom()
{
    scheduleCount = 0;
    uint8_t buf[112]; // Đủ cho MAX_SCHEDULES
    uint16_t n = 0;
    if (!eepromReadBlob(SLOT_SCHEDULE, buf, sizeof(buf), n))
        return;

    JsonDocument doc;
    if (deserializeJson(doc, buf, n) != DeserializationError::Ok)
        return;

    JsonArray arr = doc["schedules"].to<JsonArray>();
    scheduleCount = 0;
    for (JsonObject item : arr)
    {
        if (scheduleCount >= MAX_SCHEDULES)
            break;
        schedules[scheduleCount].enabled = item["enabled"] | 0;
        schedules[scheduleCount].relayIndex = item["relay"] | 1;
        schedules[scheduleCount].daysOfWeek = item["days"] | 0x7F;
        schedules[scheduleCount].hour = item["hour"] | 0;
        schedules[scheduleCount].minute = item["minute"] | 0;
        schedules[scheduleCount].action = item["action"] | 0;
        String nameStr = item["name"] | "";
        strncpy(schedules[scheduleCount].name, nameStr.c_str(), 31);
        schedules[scheduleCount].name[31] = '\0';
        scheduleCount++;
    }
}

// ================== FACTORY RESET ==================

void ConfigStore_FactoryResetAll()
{
    eepromWipeSlot(SLOT_WIFI, 256);
    eepromWipeSlot(SLOT_SERVER, 256);
    eepromWipeSlot(SLOT_SCHEDULE, 112);
    EEPROM.commit();

    // Reset về default trong RAM
    g_wifiCfg = WifiConfig();

    // Chỉ reset các giá trị cần thiết trong ServerConfig
    // String keepSerialNo = g_serverCfg.serial_no; // Lưu lại serial_no
    g_serverCfg = ServerConfig();
    // g_serverCfg.serial_no = keepSerialNo; // Khôi phục serial_no

    // Reset schedule
    scheduleCount = 0;

    // Lưu lại serial_no vào EEPROM
    ConfigStore_SaveServer();
}

// ================== SERIAL NO / DEVICE ID ==================

void ConfigStore_SetSerialNo(const String &sn)
{
    g_serverCfg.serial_no = sn;
    ConfigStore_SaveServer();
}

String ConfigStore_GetDeviceId()
{
    if (g_serverCfg.serial_no.length())
        return g_serverCfg.serial_no;

    // fallback từ chipId
    uint32_t id = ESP.getEfuseMac();
    char buf[16];
    sprintf(buf, "DVCPW%06X", (unsigned)(id & 0xFFFFFF));
    return String(buf);
}

void ConfigStore_SaveServer()
{
    JsonDocument doc;
    doc["server_ip"] = g_serverCfg.server_ip;
    doc["server_port"] = g_serverCfg.server_port;
    doc["token"] = g_serverCfg.token;
    doc["serial_no"] = g_serverCfg.serial_no;

    char buf[256];
    size_t n = serializeJson(doc, buf, sizeof(buf));
    (void)eepromWriteBlob(SLOT_SERVER, (uint8_t *)buf, (uint16_t)n);
}

void ConfigStore_SaveWifi()
{
    JsonDocument doc;
    doc["ssid"] = g_wifiCfg.ssid;
    doc["pass"] = g_wifiCfg.pass;
    doc["use_static"] = g_wifiCfg.use_static;
    doc["ip"] = g_wifiCfg.ip.toString();
    doc["mask"] = g_wifiCfg.mask.toString();
    doc["gw"] = g_wifiCfg.gw.toString();
    doc["dns1"] = g_wifiCfg.dns1.toString();
    doc["dns2"] = g_wifiCfg.dns2.toString();

    char buf[256];
    size_t n = serializeJson(doc, buf, sizeof(buf));
    (void)eepromWriteBlob(SLOT_WIFI, (uint8_t *)buf, (uint16_t)n);
}

void ConfigStore_SaveSchedule()
{
    JsonDocument doc;
    JsonArray arr = doc["schedules"].to<JsonArray>();

    for (int i = 0; i < scheduleCount; i++)
    {
        JsonObject item = arr.add<JsonObject>();
        item["enabled"] = schedules[i].enabled;
        item["relay"] = schedules[i].relayIndex;
        item["days"] = schedules[i].daysOfWeek;
        item["hour"] = schedules[i].hour;
        item["minute"] = schedules[i].minute;
        item["action"] = schedules[i].action;
        item["name"] = schedules[i].name;
    }

    char buf[512];
    size_t n = serializeJson(doc, buf, sizeof(buf));
    (void)eepromWriteBlob(SLOT_SCHEDULE, (uint8_t *)buf, (uint16_t)n);
}

void ConfigStore_Init()
{
    EEPROM.begin(EEPROM_BYTES);

    loadWifiFromEeprom();
    loadServerFromEeprom();
    loadScheduleFromEeprom();

    // Nếu serial_no trống thì tự generate rồi lưu
    if (!g_serverCfg.serial_no.length())
    {
        g_serverCfg.serial_no = ConfigStore_GetDeviceId();
        ConfigStore_SaveServer();
    }

    Serial.println(F("[CFG] ConfigStore_Init done"));
    Serial.print(F("  WiFi SSID : "));
    Serial.println(g_wifiCfg.ssid);
    Serial.print(F("  Device ID : "));
    Serial.println(ConfigStore_GetDeviceId());
    Serial.print(F("  Schedules loaded : "));
    Serial.println(scheduleCount);
}