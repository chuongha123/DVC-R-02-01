// #include "MqttService.h"

// static WiFiClient wifiClient;
// static PubSubClient mqttClient(wifiClient);
// static const char *mqttServer = "your_mqtt_server";

// static MQTTServerConfig g_serverCfg;

// static String ConfigStore_GetDeviceId();
// static void processAcCommand(const char *payload);

// static void publish(const char *topic, const char *payload);
// static void subscribe(const char *topic);

// static void callback(char *topic, byte *payload, unsigned int length);

// void mqttInit()
// {
//     mqttServer = g_serverCfg.server_ip.c_str();
//     mqttClient.setServer(mqttServer, MQTT_PORT);
//     mqttClient.setCallback(callback);
// }

// void mqttLoop()
// {
//     static unsigned long lastReconnectAttempt = 0;

//     if (!mqttClient.connected())
//     {
//         unsigned long now = millis();
//         if (now - lastReconnectAttempt > 5000)
//         {
//             lastReconnectAttempt = now;
//             if (mqttClient.connect(String(ESP.getEfuseMac()).c_str(), MQTT_USER, MQTT_PASS))
//             {
//                 subscribe(ConfigStore_GetDeviceId().c_str());
//             }
//         }
//     }
//     else
//     {
//         mqttClient.loop();
//     }
// }

// static void publish(const char *topic, const char *payload)
// {
//     mqttClient.publish(topic, payload);
// }

// static void subscribe(const char *topic)
// {
//     mqttClient.subscribe(topic);
// }

// static void callback(char *topic, byte *payload, unsigned int length)
// {
//     Serial.print(F("[MQTT] Message arrived ["));
//     Serial.print(topic);
//     Serial.print(F("] "));

//     // Chuyển payload thành string
//     String message;
//     for (unsigned int i = 0; i < length; i++)
//     {
//         message += (char)payload[i];
//     }
//     Serial.println(message);

//     // Parse JSON
//     JsonDocument doc;
//     if (deserializeJson(doc, message) != DeserializationError::Ok)
//     {
//         Serial.println(F("[MQTT] JSON parsing failed!"));
//         return;
//     }

//     // Xử lý các loại command
//     String type = doc["Type"];
//     if (type == "Setting")
//     {
//         processAcCommand(message.c_str());
//     }
//     else if (type == "Debug")
//     {
//         String debug = doc["Debug"];
//         if (debug == "WifiAP")
//         {
//         }
//         else if (debug == "ReStart")
//         {
//             ESP.restart();
//         }
//     }
// }

// static String ConfigStore_GetDeviceId()
// {
//     if (g_serverCfg.serial_no.length())
//         return g_serverCfg.serial_no;

//     // fallback từ chipId
//     uint32_t id = ESP.getEfuseMac();
//     char buf[16];
//     sprintf(buf, "DVCM0001%06X", (unsigned)(id & 0xFFFFFF));
//     return String(buf);
// }

// static void processAcCommand(const char *payload)
// {
//     JsonDocument doc;
//     if (deserializeJson(doc, payload) != DeserializationError::Ok)
//     {
//         Serial.println(F("[IR] JSON parsing failed!"));
//         return;
//     }

//     // Kiểm tra Type của command
//     String type = doc["Type"];
//     if (type != "Setting")
//         return;

//     // Lấy thông tin từ Value
//     JsonObject value = doc["Value"];
//     String status = value["Status"];
//     uint8_t setpoint = value["Temp"];
//     String fan = value["Fan"];
//     String mode = value["Mode"];
//     uint8_t swing = value["Angle"];

//     // Xử lý Mode
//     uint8_t opMode = AC_MODE_AUTO;
//     if (mode == "Cool")
//         opMode = AC_MODE_COOLING;
//     else if (mode == "Heat")
//         opMode = AC_MODE_HEATING;
//     else if (mode == "Dry")
//         opMode = AC_MODE_DRY;
//     else if (mode == "Fan")
//         opMode = AC_MODE_FAN;

//     // Xử lý Fan Speed
//     uint8_t opFanVolume = AC_FAN_VOL_LOW;
//     if (fan == "Low")
//         opFanVolume = AC_FAN_VOL_LOW;
//     else if (fan == "Medium")
//         opFanVolume = AC_FAN_VOL_MID;
//     else if (fan == "High")
//         opFanVolume = AC_FAN_VOL_HIGH;

//     // Xử lý Swing
//     uint8_t opFanDirection = AC_FAN_DIR_STOP;
//     switch (swing)
//     {
//     case 0:
//         opFanDirection = AC_FAN_DIR_STOP;
//         break;
//     default:
//         opFanDirection = AC_FAN_DIR_SWING;
//         break;
//     }

//     writeStatus(1, status == "On", opMode, opFanVolume, opFanDirection, setpoint);
// }
