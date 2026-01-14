#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H

#include <WiFi.h>
#include <PubSubClient.h>
#include "ConfigStore.h"
#include <ArduinoJson.h>

class MqttService
{
public:
    static void init();
    static void loop();
    static void publish(const char *topic, const char *payload);
    static void subscribe(const char *topic);

private:
    static void callback(char *topic, byte *payload, unsigned int length);
    static WiFiClient wifiClient;
    static PubSubClient mqttClient;
    static const char *mqttServer;
};

#endif
