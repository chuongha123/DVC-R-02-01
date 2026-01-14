#include "RestService.h"
#include "ConfigStore.h"
#include "WebService.h"

HTTPClient RestService::http;
WiFiClient RestService::client;

void RestService::init()
{
    // Khởi tạo HTTP client với timeout
    http.setTimeout(5000);
}

String RestService::get(const String &endpoint)
{
    if (!isConnected())
    {
        return "{\"error\":\"Not connected to WiFi\"}";
    }

    String url = "http://" + g_serverCfg.server_ip + ":" + String(g_serverCfg.server_port) + endpoint;
    http.begin(client, url);

    int httpCode = http.GET();
    String payload = http.getString();
    http.end();

    if (httpCode == HTTP_CODE_OK)
    {
        return payload;
    }
    return "{\"error\":\"GET request failed\"}";
}

String RestService::post(const String &endpoint, const String &body)
{
    if (!isConnected())
    {
        return "{\"error\":\"Not connected to WiFi\"}";
    }

    String url = "http://" + g_serverCfg.server_ip + ":" + String(g_serverCfg.server_port) + endpoint;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");

    int httpCode = http.POST(body);
    String payload = http.getString();
    http.end();

    if (httpCode == HTTP_CODE_OK)
    {
        return payload;
    }
    return "{\"error\":\"POST request failed\"}";
}

bool RestService::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

// void RestService::postStateRemote(const IrState &state, const String &token)
// {
//     JsonDocument doc;
//     doc["Status"] = state.power ? "On" : "Off";
//     doc["Temp"] = state.temp;
//     doc["Fan"] = String((int)state.fan);
//     doc["Hum"] = 0;
//     doc["Mode"] = String((int)state.mode);
//     doc["Angle"] = (state.swingv == stdAc::swingv_t::kAuto || state.swingh == stdAc::swingh_t::kAuto) ? 1 : 0;

//     String payload;
//     serializeJson(doc, payload);

//     if (!isConnected())
//     {
//         Serial.println("[REST] Not connected to WiFi");
//         return;
//     }

//     String url = "http://" + g_serverCfg.server_ip + ":" + g_serverCfg.server_port + "/api/device/" +
//                  ConfigStore_GetDeviceId() + "/Setting?child=0&Token=" +
//                  token;

//     http.begin(client, url);
//     http.addHeader("Content-Type", "application/json");

//     Serial.print("[REST] Update: ");
//     Serial.println(payload);

//     http.POST(payload);
//     String response = http.getString();
//     Serial.println(response);
//     http.end();
// }

String RestService::postNewGateway(const String &SN, const String &token)
{
    JsonDocument doc;
    doc["Mac"] = SN;
    doc["Name"] = "Daikin Demo Test";

    JsonObject info = doc.createNestedObject("Infor");
    info["Model"] = "DVC-R-00-01";
    info["Version"] = "0.0.1";

    doc.createNestedArray("Schedule");
    JsonArray childDevices = doc.createNestedArray("ChildDevices");

    JsonObject child = childDevices.createNestedObject();
    child["ID"] = 0;
    child["Address"] = "1";
    child["Model"] = "R";
    child["Enable"] = true;

    JsonObject measure = child.createNestedObject("Measure");
    RelayState state = getCurrentRelayState();
    JsonObject set = child.createNestedObject("Set");
    set["r1_enable"] = state.r1_enable;
    set["r1_no"] = state.r1_no;
    set["r1_nc"] = state.r1_nc;
    set["r1_control"] = state.r1_control;
    set["r2_enable"] = state.r2_enable;
    set["r2_no"] = state.r2_no;
    set["r2_nc"] = state.r2_nc;
    set["r2_control"] = state.r2_control;

    String payload;
    serializeJson(doc, payload);

    if (!isConnected())
    {
        Serial.println("[REST] Not connected to WiFi");
        return "{\"error\":\"Not connected to WiFi\"}";
    }

    String url = "http://" + g_serverCfg.server_ip + ":" + g_serverCfg.server_port + "/api/device?token=" + token;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");

    Serial.print("[REST] New Gateway: ");
    Serial.println(payload);

    http.POST(payload);
    String response = http.getString();
    Serial.println(response);
    http.end();

    return response;
}

void RestService::postDeviceSupportIR(const String &SN, const String &isSupport, const String &token) // Mặc định isSupport = "true" nếu không phải IR
{
    if (!isConnected())
    {
        Serial.println("[REST] Not connected to WiFi");
        return;
    }

    String url = "http://" + g_serverCfg.server_ip + ":" + g_serverCfg.server_port + "/api/device/" + SN + "/Setting?child=0&token=" + token;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"IsACSupport\":" + isSupport + "}";

    Serial.print("[REST] Device Support IR: ");
    Serial.println(payload);

    http.POST(payload);
    String response = http.getString();
    Serial.println(response);
    http.end();
}

void RestService::postUnixtimeAndAlive(const String &token)
{
    if (!isConnected())
    {
        Serial.println("[REST] Not connected to WiFi");
    }

    String url = "http://" + g_serverCfg.server_ip + ":" + g_serverCfg.server_port +
                 "/api/device/" + g_serverCfg.serial_no + "/Alive?token=" + token;

    http.begin(client, url);
    http.addHeader("Content-Length", "0");

    int httpCode = http.POST("");
    String payload;

    if (httpCode > 0)
    {
        payload = http.getString();
    }

    http.end();
}
