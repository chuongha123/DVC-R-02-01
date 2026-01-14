#ifndef REST_SERVICE_H
#define REST_SERVICE_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "ConfigStore.h"

class RestService
{
public:
    static void init();
    static String get(const String &endpoint);
    static String post(const String &endpoint, const String &body);
    static bool isConnected();
    // static void postStateRemote(const IrState &state, const String &token);
    static String postNewGateway(const String &SN, const String &token);
    static void postDeviceSupportIR(const String &SN, const String &isSupport, const String &token);
    static void postUnixtimeAndAlive(const String &token);

private:
    static HTTPClient http;
    static WiFiClient client;
};

#endif
