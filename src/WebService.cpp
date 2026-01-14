#include "WebService.h"
#include "RelayService.h"

// ====== CẤU HÌNH AUTH WEB ======
static const char *WEB_USER = "admin";
static const char *WEB_PASS = "admin"; // đổi lại cho phù hợp

// ====== WEBSERVER & OTA ======
static WebServer server(80);
static HTTPUpdateServer httpUpdater;

// ========== TIỆN ÍCH CHUNG ==========

static bool ensureAuth()
{
    if (!server.authenticate(WEB_USER, WEB_PASS))
    {
        server.requestAuthentication();
        return false;
    }
    return true;
}

static void sendJson(const JsonDocument &doc)
{
    String out;
    serializeJson(doc, out);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", out);
}

static bool validateAccessToken(const String &token)
{
    // Hardcode access token - bạn có thể thay đổi giá trị này
    return token == VALID_TOKEN;
}

// ========== HANDLER TRANG GỐC (HTML GZIP) ==========

static void handleRoot()
{
    if (!ensureAuth())
        return;

    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Content-Encoding", "gzip");
    server.send_P(200, "text/html", (PGM_P)relay_html_gz, relay_html_gz_len);
}

// ========== WIFI: /wifi/save (POST) ==========

static void handleWifiSave()
{
    if (!ensureAuth())
        return;

    String ssid = server.arg("ssid");
    String pass = server.arg("password");
    String ip_mode = server.arg("ip_mode"); // "dhcp" hoặc "static"

    String ipStr = server.arg("ip");
    String maskStr = server.arg("mask");
    String gwStr = server.arg("gw");
    String dns1Str = server.arg("dns1");
    String dns2Str = server.arg("dns2");

    g_wifiCfg.ssid = ssid;
    g_wifiCfg.pass = pass;

    g_wifiCfg.use_static = (ip_mode == "static");

    if (g_wifiCfg.use_static)
    {
        IPAddress tmp;

        if (tmp.fromString(ipStr))
            g_wifiCfg.ip = tmp;
        if (tmp.fromString(maskStr))
            g_wifiCfg.mask = tmp;
        if (tmp.fromString(gwStr))
            g_wifiCfg.gw = tmp;

        // DNS1
        if (dns1Str.length() && tmp.fromString(dns1Str))
        {
            g_wifiCfg.dns1 = tmp;
        }
        else
        {
            g_wifiCfg.dns1 = IPAddress(8, 8, 8, 8);
        }
        // DNS2
        if (dns2Str.length() && tmp.fromString(dns2Str))
        {
            g_wifiCfg.dns2 = tmp;
        }
        else
        {
            g_wifiCfg.dns2 = IPAddress(8, 8, 4, 4);
        }
    }
    else
    {
        // DHCP: vẫn giữ lại static cũ nếu có
    }

    ConfigStore_SaveWifi();

    // Trả về JSON response
    JsonDocument doc;
    doc["success"] = true;
    doc["message"] = "WiFi settings saved. Device will reboot now.";
    sendJson(doc);

    delay(500);
    ESP.restart();
}

// ========== SERVER CONFIG: /server/save (POST) ==========

static void handleServerSave()
{
    if (!ensureAuth())
        return;

    String token = server.arg("access_token");
    if (!validateAccessToken(token))
    {
        JsonDocument doc;
        doc["success"] = false;
        doc["message"] = "Invalid access token!";
        sendJson(doc);
        return;
    }

    String ip = server.arg("server_ip");
    String port = server.arg("server_port");
    String authToken = server.arg("token");

    g_serverCfg.server_ip = ip;
    g_serverCfg.server_port = port.toInt();
    g_serverCfg.token = authToken;

    ConfigStore_SaveServer();

    JsonDocument doc;
    doc["success"] = true;
    doc["message"] = "Server config saved. Reboot now to load new config.";
    sendJson(doc);
}

// ========== SYSTEM INFO: /system/info (GET, JSON) ==========

static void handleSystemInfo()
{
    if (!ensureAuth())
        return;

    JsonDocument d;

    d["device_id"] = ConfigStore_GetDeviceId();
    d["fw_version"] = FIRMWARE_VERSION; // hoặc FIRMWARE_VERSION
    d["current_ip"] = WiFi.localIP().toString();
    d["wifi_rssi"] = WiFi.RSSI();

    d["uptime"] = millis() / 1000;
    d["free_heap"] = ESP.getFreeHeap();
    // d["reset_reason"] = ESP.getResetReason();

    d["wifi_ssid"] = WiFi.SSID();
    d["wifi_mac"] = WiFi.macAddress();

    sendJson(d);
}

// ========== SYSTEM ACTIONS: restart, factory_reset ==========

static void handleSystemRestart()
{
    if (!ensureAuth())
        return;

    server.send(200, "text/plain", "Restarting...");
    delay(200);
    ESP.restart();
}

static void handleSystemFactoryReset()
{
    if (!ensureAuth())
        return;

    server.send(200, "text/plain", "Factory reset...");
    delay(200);
    ConfigStore_FactoryResetAll();
    delay(200);
    ESP.restart();
}

// ========== API: /api/getIP (GET) ==========

static void handleApiGetIp()
{
    if (!ensureAuth())
        return;

    JsonDocument d;
    d["ip"] = WiFi.localIP().toString();
    sendJson(d);
}

// ========== API: /api/setSerialNo?SN=IR000111 (GET) ==========

static void handleApiSetSerialNo()
{
    if (!ensureAuth())
        return;

    if (!server.hasArg("SN"))
    {
        server.send(400, "text/plain", "Missing SN parameter");
        return;
    }

    String sn = server.arg("SN");
    ConfigStore_SetSerialNo(sn);

    JsonDocument d;
    d["ok"] = true;
    d["serial_no"] = ConfigStore_GetDeviceId(); // sẽ trả về SN vừa set
    sendJson(d);
}

// In-memory relay configuration returned to UI (defaults: enabled, NO selected, control off)
static struct {
    uint8_t r1_enable = 1;
    uint8_t r1_no = 1;
    uint8_t r1_nc = 0;
    uint8_t r1_control = 0; // 0 = off, 1 = on
    uint8_t r2_enable = 1;
    uint8_t r2_no = 1;
    uint8_t r2_nc = 0;
    uint8_t r2_control = 0;
} relayCfg;

static void handleRelayInfo()
{
    if (!ensureAuth())
        return;

    JsonDocument d;
    d["r1_enable"] = relayCfg.r1_enable;
    d["r1_no"] = relayCfg.r1_no;
    d["r1_nc"] = relayCfg.r1_nc;
    // Return the user-visible control (0/1). If disabled, control is 0.
    d["r1_control"] = relayCfg.r1_enable ? relayCfg.r1_control : 0;

    d["r2_enable"] = relayCfg.r2_enable;
    d["r2_no"] = relayCfg.r2_no;
    d["r2_nc"] = relayCfg.r2_nc;
    d["r2_control"] = relayCfg.r2_enable ? relayCfg.r2_control : 0;

    sendJson(d);
}

static bool computeCoilStateFromConfig(uint8_t enable, uint8_t no, uint8_t nc, uint8_t control)
{
    // If disabled -> coil OFF (de-energized)
    if (!enable) return false;

    // If NO is selected and NC not selected: control==1 means close NO -> coil energized
    if (no && !nc) return control ? true : false;

    // If NC is selected and NO not selected: control==1 means close NC -> coil must be de-energized
    if (nc && !no) return control ? false : true;

    // If ambiguous (both 0 or both 1), fallback to NO behavior
    return control ? true : false;
}

// Accept form POST from UI to save relay config and apply control states
static void handleRelaySave()
{
    if (!ensureAuth())
        return;

    // read fields, default to previous values if missing
    relayCfg.r1_enable = server.hasArg("r1_enable") ? server.arg("r1_enable").toInt() : relayCfg.r1_enable;
    relayCfg.r1_no = server.hasArg("r1_no") ? server.arg("r1_no").toInt() : relayCfg.r1_no;
    relayCfg.r1_nc = server.hasArg("r1_nc") ? server.arg("r1_nc").toInt() : relayCfg.r1_nc;
    relayCfg.r1_control = server.hasArg("r1_control") ? server.arg("r1_control").toInt() : relayCfg.r1_control;

    relayCfg.r2_enable = server.hasArg("r2_enable") ? server.arg("r2_enable").toInt() : relayCfg.r2_enable;
    relayCfg.r2_no = server.hasArg("r2_no") ? server.arg("r2_no").toInt() : relayCfg.r2_no;
    relayCfg.r2_nc = server.hasArg("r2_nc") ? server.arg("r2_nc").toInt() : relayCfg.r2_nc;
    relayCfg.r2_control = server.hasArg("r2_control") ? server.arg("r2_control").toInt() : relayCfg.r2_control;

    // Compute coil states from config and apply to hardware
    bool coil1 = computeCoilStateFromConfig(relayCfg.r1_enable, relayCfg.r1_no, relayCfg.r1_nc, relayCfg.r1_control);
    bool coil2 = computeCoilStateFromConfig(relayCfg.r2_enable, relayCfg.r2_no, relayCfg.r2_nc, relayCfg.r2_control);

    RelayService_Set(1, coil1);
    RelayService_Set(2, coil2);

    // If disabled, ensure the user-visible control is 0 (UI will read this)
    if (!relayCfg.r1_enable) relayCfg.r1_control = 0;
    if (!relayCfg.r2_enable) relayCfg.r2_control = 0;

    JsonDocument d;
    d["ok"] = true;
    d["message"] = "Relay config saved.";
    d["applied_r1_coil"] = coil1;
    d["applied_r2_coil"] = coil2;
    sendJson(d);
}

// Keep the lightweight /relay/set for direct control from other UIs
static void handleRelaySet()
{
    if (!ensureAuth())
        return;

    if (!server.hasArg("relay") || !server.hasArg("state"))
    {
        server.send(400, "text/plain", "Missing parameters (relay, state)");
        return;
    }

    int r = server.arg("relay").toInt();
    String s = server.arg("state");
    bool on = (s == "on" || s == "1");

    // Enforce enable flag: if disabled, reject control attempts
    if (r == 1 && !relayCfg.r1_enable) {
        server.send(400, "text/plain", "Relay 1 is disabled");
        return;
    }
    if (r == 2 && !relayCfg.r2_enable) {
        server.send(400, "text/plain", "Relay 2 is disabled");
        return;
    }

    // Update user-visible control and compute coil based on NO/NC selection
    if (r == 1) {
        relayCfg.r1_control = on ? 1 : 0;
        bool coil = computeCoilStateFromConfig(relayCfg.r1_enable, relayCfg.r1_no, relayCfg.r1_nc, relayCfg.r1_control);
        RelayService_Set(1, coil);
    }
    if (r == 2) {
        relayCfg.r2_control = on ? 1 : 0;
        bool coil = computeCoilStateFromConfig(relayCfg.r2_enable, relayCfg.r2_no, relayCfg.r2_nc, relayCfg.r2_control);
        RelayService_Set(2, coil);
    }

    JsonDocument d;
    d["ok"] = true;
    d["relay"] = r;
    d["state"] = (r == 1) ? (relayCfg.r1_control == 1) : (relayCfg.r2_control == 1);
    sendJson(d);
}

static void handleServerInfo()
{
    if (!ensureAuth())
        return;

    JsonDocument d;
    d["server_ip"] = g_serverCfg.server_ip;
    d["server_port"] = g_serverCfg.server_port;
    d["access_token"] = g_serverCfg.token;
    sendJson(d);
}

// ========== GET CURRENT RELAY STATE ==========

RelayState getCurrentRelayState()
{
    RelayState state;
    state.r1_enable = relayCfg.r1_enable;
    state.r1_no = relayCfg.r1_no;
    state.r1_nc = relayCfg.r1_nc;
    state.r1_control = relayCfg.r1_control;
    state.r2_enable = relayCfg.r2_enable;
    state.r2_no = relayCfg.r2_no;
    state.r2_nc = relayCfg.r2_nc;
    state.r2_control = relayCfg.r2_control;
    return state;
}

// ========== PUBLIC API ==========

void WebService_Begin()
{
    // Route HTML chính
    server.on("/", HTTP_GET, handleRoot);

    // WiFi
    server.on("/wifi/save", HTTP_POST, handleWifiSave);

    // Relay
    server.on("/relay/info", HTTP_GET, handleRelayInfo);
    server.on("/relay/save", HTTP_POST, handleRelaySave);
    server.on("/relay/set", HTTP_POST, handleRelaySet);

    // Server
    server.on("/server/save", HTTP_POST, handleServerSave);
    server.on("/server/info", HTTP_GET, handleServerInfo);

    // System
    server.on("/system/info", HTTP_GET, handleSystemInfo);
    server.on("/system/restart", HTTP_POST, handleSystemRestart);
    server.on("/system/factory_reset", HTTP_POST, handleSystemFactoryReset);

    // API
    server.on("/api/getIP", HTTP_GET, handleApiGetIp);
    server.on("/api/setSerialNo", HTTP_GET, handleApiSetSerialNo);

    // OTA firmware: /system/update (cũng yêu cầu Basic Auth)
    httpUpdater.setup(&server, "/system/update", WEB_USER, WEB_PASS);

    server.begin();
}

void WebService_Handle()
{
    server.handleClient();
}