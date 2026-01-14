#include "MqttService.h"

WiFiClient MqttService::wifiClient;
PubSubClient MqttService::mqttClient(wifiClient);
const char *MqttService::mqttServer = "your_mqtt_server";

void MqttService::init()
{
    mqttServer = g_serverCfg.server_ip.c_str();
    mqttClient.setServer(mqttServer, MQTT_PORT);
    mqttClient.setCallback(callback);
}

void MqttService::loop()
{
    static unsigned long lastReconnectAttempt = 0;

    if (!mqttClient.connected())
    {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > 5000)
        {
            lastReconnectAttempt = now;
            if (mqttClient.connect(String(ESP.getEfuseMac()).c_str(), MQTT_USER, MQTT_PASS))
            {
                subscribe(ConfigStore_GetDeviceId().c_str());
            }
        }
    }
    else
    {
        mqttClient.loop();
    }
}

void MqttService::publish(const char *topic, const char *payload)
{
    mqttClient.publish(topic, payload);
}

void MqttService::subscribe(const char *topic)
{
    mqttClient.subscribe(topic);
}

void MqttService::callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print(F("[MQTT] Message arrived ["));
    Serial.print(topic);
    Serial.print(F("] "));

    // Chuyển payload thành string
    String message;
    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }
    Serial.println(message);

    // Parse JSON
    JsonDocument doc;
    if (deserializeJson(doc, message) != DeserializationError::Ok)
    {
        Serial.println(F("[MQTT] JSON parsing failed!"));
        return;
    }

    // Xử lý các loại command
    String type = doc["Type"];
    if (type == "Setting")
    {
        // Setting command ignored (AC support removed)
    }
    else if (type == "Debug")
    {
        String debug = doc["Debug"];
        if (debug == "WifiAP")
        {
        }
        else if (debug == "ReStart")
        {
            ESP.restart();
        }
    }
}

