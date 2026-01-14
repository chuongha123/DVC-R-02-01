// #pragma once
// #include <ETH.h>
// #include <WebServer.h>
// #include "Preferences.h"
// #include "ControlAC.h"
// #include "ArduinoJson.h"

// // Define the Ethernet address
// #define ETH_ADDR 0
// #define ETH_POWER_PIN -1
// #define ETH_MDC_PIN 23
// #define ETH_MDIO_PIN 18
// #define ETH_TYPE ETH_PHY_LAN8720
// #define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT



// enum NetworkMode
// {
//     ETHERNET,
//     WIFI,
//     AP
// };

// struct AuthInfo
// {
//     String ssid;
//     String password;
// };

// struct NetworkInfo
// {
//     IPAddress ip;
//     IPAddress gateway;
//     IPAddress subnet;
//     bool dhcp;
//     NetworkMode networkMode;
//     AuthInfo authWifiInfo;
//     AuthInfo authAPInfo;
// };

// struct ACStatus
// {
//     bool power;
//     uint8_t mode;
//     uint8_t fanDirection;
//     uint8_t fanVolume;
//     int16_t setpoint;
// };


// void handleStatus();
// void handleControl();

// void sendConfig();
// void handleNetworkConfig();
// void clearNetworkConfigHandle();
// void saveStaticNetworkConfig(IPAddress ip, IPAddress gw, IPAddress sn);
// void loadNetworkConfig();
// void restartDevice();

// void initEthernet();
// void initWifi();
// void initAP();

// void restAPIBegin();
// void restAPILoop();
// void saveNetWorkConfig(JsonObject obj);

// void clearNetworkConfig();
