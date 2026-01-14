#pragma once
#include "WiFi.h"
#include <WebServer.h>
#include "HTTPUpdateServer.h"
#include "ConfigStore.h"
// HTML gzip từ file bạn build (nội dung 4 tab WiFi / IR / Server / System)
#include "WebUiGzip.h" // định nghĩa index_html_gz & index_html_gz_len

struct RelayState
{
    uint8_t r1_enable;
    uint8_t r1_no;
    uint8_t r1_nc;
    uint8_t r1_control;
    uint8_t r2_enable;
    uint8_t r2_no;
    uint8_t r2_nc;
    uint8_t r2_control;
};

RelayState getCurrentRelayState();

void WebService_Begin();
void WebService_Handle();