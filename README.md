# DVC-R-02-01 – ESP32 Relay Control Gateway

Firmware cho ESP32 dùng để điều khiển relay qua Web UI, REST API và MQTT. Dự án được cấu hình bằng PlatformIO, framework Arduino.

## 📋 Mục lục

- [Tổng quan](#tổng-quan)
- [Chức năng chính](#chức-năng-chính)
- [Phần cứng](#phần-cứng)
- [Cài đặt](#cài-đặt)
- [Cấu hình](#cấu-hình)
- [API Reference](#api-reference)
- [Cấu trúc dự án](#cấu-trúc-dự-án)
- [Build & Upload](#build--upload)
- [Nguyên lý hoạt động](#nguyên-lý-hoạt-động)
- [Troubleshooting](#troubleshooting)

## 🎯 Tổng quan

Dự án này cung cấp một gateway IoT để điều khiển relay trên ESP32. Hệ thống hỗ trợ:

- Điều khiển 2 relay độc lập (GPIO 15 và GPIO 2)
- Web UI để cấu hình và điều khiển relay real-time
- REST API để tích hợp với hệ thống khác
- MQTT để nhận lệnh điều khiển từ server
- Lưu cấu hình vào EEPROM
- OTA firmware update
- Factory reset bằng nút nhấn

## ⚡ Chức năng chính

### 1. Điều khiển Relay
- Hỗ trợ 2 relay độc lập
- Cấu hình NO (Normally Open) / NC (Normally Closed)
- Bật/tắt từng relay độc lập
- Enable/disable từng relay
- Hỗ trợ relay module active LOW hoặc active HIGH

### 2. Web UI
- Giao diện cấu hình WiFi, Relay, Server, System
- Tab Relay hiển thị và điều khiển trạng thái relay real-time
- Cấu hình NO/NC cho từng relay
- Enable/disable relay
- Auto-refresh trạng thái

### 3. REST API
- `GET /relay/info` - Lấy thông tin cấu hình và trạng thái relay
- `POST /relay/save` - Lưu cấu hình relay (enable, NO/NC, control)
- `POST /relay/set?relay=1&state=on` - Điều khiển relay trực tiếp
- Gửi thông tin gateway lên server qua `RestService::postNewGateway()`
- Gửi alive/unixtime mỗi 5 phút

### 4. MQTT
- Subscribe topic `device/cmd` để nhận lệnh điều khiển
- Publish thông tin device lên topic `device/info`
- Tích hợp với MQTT broker

### 5. WiFi & Cấu hình
- Lưu cấu hình WiFi, Server, Serial/Device ID vào EEPROM
- Hỗ trợ AP fallback khi chưa cấu hình WiFi
- Hỗ trợ DHCP và Static IP
- Factory reset bằng nút nhấn (giữ 5 giây ở GPIO 5)

## 🔌 Phần cứng

### Yêu cầu
- **Board**: ESP32 (ESP32-DevKitC hoặc tương đương)
- **Relay Module**: 2 kênh relay module (active LOW hoặc active HIGH)
- **Nút nhấn**: 1 nút nhấn cho factory reset (tùy chọn)

### Sơ đồ kết nối

```
Relay Module          ESP32
─────────────────     ────────
VCC  ──────────────>  3.3V hoặc 5V (tùy relay module)
GND  ──────────────>  GND
IN1  ──────────────>  GPIO 15 (Relay 1)
IN2  ──────────────>  GPIO 2  (Relay 2)

Nút Factory Reset:
Button ────────────>  GPIO 5
      │
      └──> GND (khi nhấn)
```

**Lưu ý**: 
- Kiểm tra relay module của bạn là active LOW hay active HIGH
- Nếu active LOW, relay sẽ bật khi GPIO = LOW
- Nếu active HIGH, relay sẽ bật khi GPIO = HIGH

### Pin Configuration

| Chức năng | Pin ESP32 | GPIO | Ghi chú |
|-----------|-----------|------|---------|
| Relay 1 | GPIO 15 | 15 | Điều khiển relay 1 |
| Relay 2 | GPIO 2 | 2 | Điều khiển relay 2 |
| Factory Reset | GPIO 5 | 5 | Nhấn giữ 5s |

## 📦 Cài đặt

### Yêu cầu
- VS Code + PlatformIO extension, hoặc PlatformIO Core (CLI)
- USB cable để kết nối ESP32

### Thư viện phụ thuộc

Các thư viện được cấu hình tự động trong `platformio.ini`:

- `4-20ma/ModbusMaster@^2.0.1` - Modbus communication (nếu cần)
- `bblanchon/ArduinoJson@^7.4.2` - Xử lý JSON
- `knolleary/PubSubClient@^2.8` - MQTT client

## ⚙️ Cấu hình

### 1. Cấu hình Pin Relay

Mở `include/ConfigStore.h`:

```cpp
// Relay pins (GPIO numbers) - Mảng để dễ dàng mở rộng
const int RELAY_PINS[] = {15, 2};
const int NUM_RELAYS = sizeof(RELAY_PINS) / sizeof(RELAY_PINS[0]);
```

**Thay đổi pin**: Sửa mảng `RELAY_PINS[]` để thay đổi pin relay

**Thêm relay mới**: Thêm pin vào mảng `RELAY_PINS[]` (ví dụ: `{15, 2, 4}`)

### 2. Cấu hình Relay Active Level

Mở `src/RelayService.cpp`, dòng 4-7:

```cpp
// If your relay module is active LOW, set RELAY_ACTIVE_LOW to 1
#ifndef RELAY_ACTIVE_LOW
#define RELAY_ACTIVE_LOW 0  // 0 = active HIGH, 1 = active LOW
#endif
```

**Thay đổi**: Đặt `RELAY_ACTIVE_LOW` thành `1` nếu relay module của bạn là active LOW

### 3. Cấu hình Server/MQTT

Mở `include/ConfigStore.h`:

```cpp
#define SERVER_PORT 8080
#define SERVER_TOKEN "your_token_here"
#define MQTT_PORT 1883
#define MQTT_USER "ViotBroker"
#define MQTT_PASS "Viot123!"
```

### 4. Cấu hình Web Authentication

Mở `src/WebService.cpp`, dòng 5-6:

```cpp
static const char *WEB_USER = "admin";
static const char *WEB_PASS = "admin"; // đổi lại cho phù hợp
```

**Thay đổi**: Đổi username và password cho Web UI

### 5. Cấu hình thời gian gửi alive

Mở `src/main.cpp`, dòng 66:

```cpp
TimerService::setInterval(300000, []() { ... }); // 5 phút (300000ms)
```

**Thay đổi**: Đổi `300000` thành giá trị khác (milliseconds) để thay đổi tần suất gửi alive

## 📡 API Reference

### Web API

#### GET /relay/info
Lấy thông tin cấu hình và trạng thái relay

**Response**:
```json
{
  "r1_enable": 1,
  "r1_no": 1,
  "r1_nc": 0,
  "r1_control": 0,
  "r2_enable": 1,
  "r2_no": 1,
  "r2_nc": 0,
  "r2_control": 0
}
```

**Giải thích**:
- `r1_enable`, `r2_enable`: 1 = enabled, 0 = disabled
- `r1_no`, `r2_no`: 1 = NO (Normally Open) được chọn
- `r1_nc`, `r2_nc`: 1 = NC (Normally Closed) được chọn
- `r1_control`, `r2_control`: 1 = ON, 0 = OFF (trạng thái điều khiển)

#### POST /relay/save
Lưu cấu hình relay

**Parameters** (form data):
- `r1_enable`: 0 hoặc 1
- `r1_no`: 0 hoặc 1
- `r1_nc`: 0 hoặc 1
- `r1_control`: 0 hoặc 1
- `r2_enable`: 0 hoặc 1
- `r2_no`: 0 hoặc 1
- `r2_nc`: 0 hoặc 1
- `r2_control`: 0 hoặc 1

**Response**:
```json
{
  "ok": true,
  "message": "Relay config saved.",
  "applied_r1_coil": false,
  "applied_r2_coil": false
}
```

#### POST /relay/set
Điều khiển relay trực tiếp

**Parameters** (query string hoặc form data):
- `relay`: 1 hoặc 2
- `state`: "on", "off", "1", hoặc "0"

**Response**:
```json
{
  "ok": true,
  "relay": 1,
  "state": 1
}
```

#### GET /api/getIP
Lấy địa chỉ IP hiện tại

**Response**:
```json
{
  "ip": "192.168.1.100"
}
```

#### GET /api/setSerialNo?SN=IR000111
Đặt Serial Number / Device ID

**Response**:
```json
{
  "ok": true,
  "serial_no": "IR000111"
}
```

### REST API (Gửi lên server)

#### postNewGateway()
Gửi thông tin gateway lên server

**Payload**:
```json
{
  "Mac": "DVCPW123456",
  "Name": "Daikin Demo Test",
  "Infor": {
    "Model": "DVC-R-02-01",
    "Version": "3.0.3"
  },
  "ChildDevices": [...]
}
```

## 📁 Cấu trúc dự án

```
DVC-R-02-01/
├── include/                    # Header files
│   ├── ConfigStore.h          # Cấu hình chung (pin, server, MQTT)
│   ├── RelayService.h         # API điều khiển relay
│   ├── WebService.h           # Web server & API endpoints
│   ├── WifiService.h          # WiFi management
│   ├── MqttService.h          # MQTT client
│   ├── RestService.h          # REST API client
│   └── TimerService.h         # Timer utilities
│
├── src/                        # Source files
│   ├── main.cpp               # Entry point, khởi tạo các service
│   ├── RelayService.cpp       # Logic điều khiển relay
│   ├── WebService.cpp         # HTTP server, API handlers
│   ├── WifiService.cpp        # WiFi STA/AP mode
│   ├── MqttService.cpp        # MQTT publish/subscribe
│   ├── RestService.cpp        # REST API calls
│   ├── ConfigStore.cpp        # EEPROM read/write
│   ├── TimerService.cpp       # Timer utilities
│   └── WebUiGzip.cpp          # Embedded HTML (gzip)
│
├── test/                       # Test files
│   └── RelayService_Test.cpp  # Test cases
│
├── Relay.html                  # Web UI HTML (chưa gzip)
├── relay.html.gz               # Web UI HTML (gzip)
├── platformio.ini              # PlatformIO configuration
└── README.md                   # File này
```

## 🔨 Build & Upload

### Sử dụng PlatformIO CLI

```bash
# Build project
pio run

# Upload firmware
pio run -t upload

# Monitor serial
pio device monitor -b 115200

# Clean build
pio run -t clean
```

### Sử dụng VS Code

1. Mở project trong VS Code
2. Click nút **Build** (✓) trên thanh PlatformIO
3. Click nút **Upload** (→) để upload firmware
4. Click nút **Monitor** (🔌) để xem Serial output

### Cấu hình Board

File `platformio.ini`:
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
```

**Nếu dùng board khác**, đổi `board = esp32dev` thành:
- `esp32doit-devkit-v1` - DOIT ESP32 DevKit V1
- `nodemcu-32s` - NodeMCU-32S
- `lolin32` - WEMOS LOLIN32

## 🔬 Nguyên lý hoạt động

### 1. Quy trình điều khiển Relay

Relay được điều khiển qua GPIO với logic:

#### Active HIGH (mặc định):
- GPIO = HIGH → Relay ON (coil energized)
- GPIO = LOW → Relay OFF (coil de-energized)

#### Active LOW:
- GPIO = LOW → Relay ON (coil energized)
- GPIO = HIGH → Relay OFF (coil de-energized)

### 2. Cấu hình NO/NC

Hệ thống hỗ trợ cấu hình NO (Normally Open) và NC (Normally Closed):

- **NO được chọn**: Khi control = 1, relay đóng tiếp điểm NO → coil phải energized
- **NC được chọn**: Khi control = 1, relay đóng tiếp điểm NC → coil phải de-energized

Logic tính toán coil state:
```cpp
bool computeCoilStateFromConfig(bool enable, bool no, bool nc, bool control) {
    if (!enable) return false;  // Disabled → coil OFF
    
    if (no && !nc) return control ? true : false;   // NO: control=1 → coil ON
    if (nc && !no) return control ? false : true;   // NC: control=1 → coil OFF
    return control ? true : false;  // Fallback to NO behavior
}
```

### 3. Luồng dữ liệu

```
User/Server
    │
    ├─> Web UI (HTTP)
    │   └─> WebService
    │       └─> RelayService_Set()
    │
    ├─> REST API (HTTP)
    │   └─> WebService
    │       └─> RelayService_Set()
    │
    └─> MQTT
        └─> MqttService
            └─> RelayService_Set()
                    │
                    └─> GPIO (ESP32)
                            │
                            └─> Relay Module
```

### 4. Khởi tạo hệ thống

1. **Setup()** (`src/main.cpp`):
   - Khởi tạo Serial (115200 baud)
   - Khởi tạo Relay: `RelayService_Begin()` (set GPIO mode, default OFF)
   - Khởi tạo EEPROM: `ConfigStore_Init()`
   - Khởi động WiFi (STA hoặc AP mode)
   - Khởi động Web Server: `WebService_Begin()`
   - Khởi động MQTT: `MqttService::init()`
   - Gửi thông tin gateway lên server
   - Setup timer gửi alive mỗi 5 phút

2. **Loop()**:
   - Xử lý HTTP requests
   - Xử lý MQTT messages
   - Xử lý timer events
   - Kiểm tra factory reset button

### 5. Factory Reset

- Nhấn và giữ nút ở GPIO 5 trong 5 giây
- Hệ thống sẽ xóa toàn bộ cấu hình (WiFi, Server, SerialNo)
- ESP32 sẽ tự động restart

## 🐛 Troubleshooting

### Relay không hoạt động

**Triệu chứng**: Relay không bật/tắt khi điều khiển

**Nguyên nhân có thể**:
1. Relay module active level không đúng (LOW vs HIGH)
2. Pin kết nối sai (kiểm tra `RELAY_PINS[]` trong `ConfigStore.h`)
3. Relay module chưa được cấp nguồn đúng
4. Relay bị hỏng

**Giải pháp**:
- Kiểm tra cấu hình `RELAY_ACTIVE_LOW` trong `RelayService.cpp`
- Kiểm tra kết nối VCC, GND, IN1, IN2
- Kiểm tra bằng multimeter hoặc LED test
- Thử đổi pin khác

### Web UI không truy cập được

**Nguyên nhân**: WiFi chưa kết nối hoặc địa chỉ IP sai

**Giải pháp**:
- Kiểm tra Serial monitor để xem IP address
- Nếu chưa cấu hình WiFi, kết nối vào AP mode (SSID = SerialNo)
- Kiểm tra username/password (mặc định: admin/admin)

### MQTT không kết nối

**Nguyên nhân**: Thông tin MQTT broker sai hoặc network issue

**Giải pháp**:
- Kiểm tra `MQTT_USER`, `MQTT_PASS`, `MQTT_PORT` trong `ConfigStore.h`
- Kiểm tra MQTT broker có hoạt động không
- Kiểm tra firewall/network

### Build lỗi

**Lỗi thường gặp**:
- Library không tìm thấy: Chạy `pio pkg install`
- Board không nhận diện: Kiểm tra `platformio.ini` và board selection
- Port không tìm thấy: Kiểm tra USB cable và driver ESP32

## 📝 Changelog

### Version 3.0.3
- ✅ ESP32 Relay Control với 2 relay
- ✅ Web UI điều khiển relay real-time
- ✅ REST API `/relay/info`, `/relay/save`, `/relay/set`
- ✅ MQTT support
- ✅ Factory reset bằng nút nhấn
- ✅ OTA firmware update
- ✅ Cấu hình NO/NC cho từng relay

## 📄 License

[Thêm license của bạn ở đây]

## 👤 Author

[Thêm thông tin tác giả]

## 🙏 Acknowledgments

- ArduinoJson library by Benoit Blanchon
- PubSubClient library by Nick O'Leary
- ESP32 Arduino Core
#   D V C - R - 0 2 - 0 1 - V E D G E  
 