# DVC-T-00-01 – ESP8266 DS18B20 Temperature Gateway

Firmware cho ESP8266 dùng làm gateway đọc nhiệt độ từ cảm biến DS18B20 qua Web UI, REST API và MQTT. Dự án được cấu hình bằng PlatformIO, framework Arduino.

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

Dự án này cung cấp một gateway IoT để đọc và hiển thị nhiệt độ từ cảm biến DS18B20 trên ESP8266. Hệ thống hỗ trợ:

- Đọc nhiệt độ từ tối đa 2 cảm biến DS18B20
- Web UI để xem nhiệt độ real-time
- REST API để tích hợp với hệ thống khác
- MQTT để publish/subscribe dữ liệu nhiệt độ
- Lưu cấu hình vào EEPROM
- OTA firmware update

## ⚡ Chức năng chính

### 1. Đọc nhiệt độ DS18B20
- Hỗ trợ tối đa 2 cảm biến DS18B20 trên cùng 1 bus OneWire
- Độ phân giải: 12-bit (0.0625°C)
- Phạm vi: -55°C đến +125°C
- Độ chính xác: ±0.5°C (0-70°C)

### 2. Web UI
- Giao diện cấu hình WiFi, Server, System
- Tab Temperature hiển thị nhiệt độ real-time
- Auto-refresh mỗi 2 giây
- Hiển thị trạng thái kết nối của từng sensor

### 3. REST API
- `GET /temperature/info` - Lấy thông tin tất cả sensors
- `GET /temperature/get?sensor=1` - Lấy nhiệt độ sensor cụ thể
- Gửi dữ liệu nhiệt độ lên server qua `RestService::postNewGateway()`
- Gửi alive/unixtime mỗi 5 phút

### 4. MQTT
- Subscribe topic để nhận lệnh điều khiển
- Publish dữ liệu nhiệt độ (có thể mở rộng)

### 5. WiFi & Cấu hình
- Lưu cấu hình WiFi, Server, Serial/Device ID vào EEPROM
- Hỗ trợ AP fallback khi chưa cấu hình WiFi
- Factory reset bằng nút nhấn (giữ 5 giây)

## 🔌 Phần cứng

### Yêu cầu
- **Board**: ESP8266 (NodeMCU v2 hoặc tương đương)
- **Cảm biến**: DS18B20 (tối đa 2 cảm biến)
- **Điện trở**: 4.7kΩ pull-up (giữa DQ và VCC)

### Sơ đồ kết nối

```
DS18B20 Sensor(s)          ESP8266
─────────────────          ────────
VCC  ────────────────>  3.3V
GND  ────────────────>  GND
DQ   ────────────────>  D2 (GPIO 4)
      │
      └──[4.7kΩ]──> 3.3V (Pull-up resistor)
```

**Lưu ý**: 
- Nếu dùng nhiều cảm biến, tất cả VCC, GND, DQ nối chung (bus OneWire)
- Điện trở pull-up 4.7kΩ chỉ cần 1 cái cho cả bus

### Pin Configuration

| Chức năng | Pin ESP8266 | GPIO | Ghi chú |
|-----------|-------------|------|---------|
| DS18B20 Data | D2 | GPIO 4 | OneWire bus |
| Factory Reset | D1 | GPIO 5 | Nhấn giữ 5s |

## 📦 Cài đặt

### Yêu cầu
- VS Code + PlatformIO extension, hoặc PlatformIO Core (CLI)
- USB cable để kết nối ESP8266

### Thư viện phụ thuộc

Các thư viện được cấu hình tự động trong `platformio.ini`:

- `OneWire@^2.3.7` - Giao tiếp OneWire protocol
- `milesburton/DallasTemperature` - Thư viện cho DS18B20
- `ArduinoJson@^7.4.2` - Xử lý JSON
- `PubSubClient@^2.8` - MQTT client
- `ModbusMaster@^2.0.1` - Modbus (nếu cần)

## ⚙️ Cấu hình

### 1. Cấu hình Pin cảm biến

Mở `include/ConfigStore.h`:

```cpp
// DS18B20 Temperature Sensor pin (OneWire bus) - ESP8266 D2 = GPIO 4
#define DS18_PIN D2
#define MAX_DS18_SENSORS 2  // Maximum number of DS18B20 sensors supported
```

**Thay đổi pin**: Đổi `D2` thành pin khác nếu cần (ví dụ: `D4`, `D5`)

**Thay đổi số lượng sensor**: Đổi `MAX_DS18_SENSORS` (hiện tại hỗ trợ tối đa 2)

### 2. Cấu hình Resolution

Mở `src/TemperatureService.cpp`, dòng 48:

```cpp
sensors.setResolution(tempDeviceAddress[i], 12);  // 9, 10, 11, hoặc 12
```

**Các mức resolution**:
- **9-bit**: ~94ms, độ phân giải 0.5°C
- **10-bit**: ~188ms, độ phân giải 0.25°C
- **11-bit**: ~375ms, độ phân giải 0.125°C
- **12-bit**: ~750ms, độ phân giải 0.0625°C (mặc định)

### 3. Cấu hình Server/MQTT

Mở `include/ConfigStore.h`:

```cpp
#define SERVER_PORT 8080
#define SERVER_TOKEN "your_token_here"
#define MQTT_PORT 1883
#define MQTT_USER "ViotBroker"
#define MQTT_PASS "Viot123!"
```

### 4. Cấu hình thời gian cập nhật

**Auto-refresh Web UI**: Mở `Temp.html` (hoặc `Relay.html`), dòng 777:
```javascript
autoRefreshInterval = setInterval(updateTemperatureDisplay, 2000); // 2 giây
```

**Gửi alive lên server**: Mở `src/main.cpp`, dòng 66:
```cpp
TimerService::setInterval(300000, []() { ... }); // 5 phút (300000ms)
```

**Delay đọc nhiệt độ**: Mở `src/WebService.cpp`, dòng 245:
```cpp
delay(100); // Nên tăng lên 750-1000ms cho resolution 12-bit
```

## 📡 API Reference

### Web API

#### GET /temperature/info
Lấy thông tin tất cả sensors

**Response**:
```json
{
  "sensor_count": 2,
  "temp1": 25.5,
  "sensor1_connected": true,
  "temp1_valid": true,
  "temp2": 23.2,
  "sensor2_connected": true,
  "temp2_valid": true
}
```

#### GET /temperature/get?sensor=1
Lấy nhiệt độ sensor cụ thể

**Parameters**:
- `sensor`: 1 hoặc 2

**Response**:
```json
{
  "ok": true,
  "sensor": 1,
  "temperature": 25.5,
  "connected": true,
  "valid": true
}
```

### REST API (Gửi lên server)

#### postNewGateway()
Gửi thông tin gateway và dữ liệu nhiệt độ lên server

**Payload**:
```json
{
  "Mac": "DVCPW123456",
  "Name": "Daikin Demo Test",
  "Infor": {
    "Model": "DVC-R-00-01",
    "Version": "0.0.1"
  },
  "ChildDevices": [{
    "ID": 0,
    "Address": "1",
    "Model": "R",
    "Enable": true,
    "Set": {
      "temp1": 25.5,
      "temp2": 23.2,
      "sensor1_connected": true,
      "sensor2_connected": true,
      "sensor_count": 2
    }
  }]
}
```

## 📁 Cấu trúc dự án

```
DVC-T-00-01/
├── include/                    # Header files
│   ├── ConfigStore.h          # Cấu hình chung (pin, server, MQTT)
│   ├── TemperatureService.h   # API đọc nhiệt độ DS18B20
│   ├── WebService.h           # Web server & API endpoints
│   ├── WifiService.h          # WiFi management
│   ├── MqttService.h          # MQTT client
│   └── RestService.h          # REST API client
│
├── src/                        # Source files
│   ├── main.cpp               # Entry point, khởi tạo các service
│   ├── TemperatureService.cpp # Logic đọc DS18B20
│   ├── WebService.cpp         # HTTP server, API handlers
│   ├── WifiService.cpp        # WiFi STA/AP mode
│   ├── MqttService.cpp        # MQTT publish/subscribe
│   ├── RestService.cpp        # REST API calls
│   ├── ConfigStore.cpp        # EEPROM read/write
│   ├── TimerService.cpp       # Timer utilities
│   └── WebUiGzip.cpp          # Embedded HTML (gzip)
│
├── test/                       # Test files
│   └── RelayService_Test.cpp  # Test cases (cũ, có thể xóa)
│
├── Temp.html                   # Web UI HTML (chưa gzip)
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
[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
framework = arduino
```

**Nếu dùng board khác**, đổi `board = nodemcuv2` thành:
- `d1_mini` - Wemos D1 Mini
- `d1_mini_lite` - Wemos D1 Mini Lite
- `esp01` - ESP-01
- `esp01_1m` - ESP-01 1MB

## 🔬 Nguyên lý hoạt động

### 1. Quy trình đọc nhiệt độ

DS18B20 sử dụng giao thức **OneWire** với quy trình 2 bước:

#### Bước 1: Request Conversion (Yêu cầu đo)
```cpp
TemperatureService_RequestTemperatures();
// Gửi lệnh qua OneWire để cảm biến bắt đầu đo nhiệt độ
// Cảm biến tự đo và lưu vào bộ nhớ (750ms cho 12-bit)
```

#### Bước 2: Read Scratchpad (Đọc giá trị)
```cpp
float temp = TemperatureService_GetTemperature(1);
// Đọc dữ liệu từ bộ nhớ cảm biến
// Chuyển đổi từ raw 16-bit value sang độ C
```

### 2. Bản chất giá trị cảm biến

DS18B20 lưu nhiệt độ dưới dạng **16-bit signed integer** trong Scratchpad:

- **Format**: 2 bytes (LSB + MSB)
- **Resolution 12-bit**: Độ phân giải 0.0625°C (1/16°C)
- **Công thức**: `Temperature (°C) = raw_value / 16.0`

**Ví dụ**:
- Raw value: `0x0190` (400 decimal) → 400/16 = **25.0°C**
- Raw value: `0x0198` (408 decimal) → 408/16 = **25.5°C**
- Raw value: `0xFF60` (-160 decimal) → -160/16 = **-10.0°C**

**Cấu trúc 16-bit**:
```
Bit:  15  14  13  12  11  10  9  8  7  6  5  4  3  2  1  0
      S   S   S   S   S   2^3 2^2 2^1 2^0 2^-1 2^-2 2^-3 2^-4
      (Sign)  (Integer part)    (Fractional part)
```

### 3. Luồng dữ liệu

```
DS18B20 Sensor
    │
    │ [OneWire Bus - Pin D2]
    │
ESP8266
    │
    ├─> TemperatureService (đọc raw value)
    │
    ├─> WebService (hiển thị trên Web UI)
    │   └─> Auto-refresh mỗi 2 giây
    │
    ├─> RestService (gửi lên server)
    │   └─> Mỗi 5 phút gửi alive + dữ liệu
    │
    └─> MqttService (publish MQTT - có thể mở rộng)
```

### 4. Khởi tạo hệ thống

1. **Setup()** (`src/main.cpp`):
   - Khởi tạo Serial (115200 baud)
   - Khởi tạo DS18B20: `TemperatureService_Begin()`
     - Tìm sensors trên bus
     - Lưu địa chỉ unique của mỗi sensor
     - Cấu hình resolution 12-bit
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

## 🐛 Troubleshooting

### Không tìm thấy cảm biến

**Triệu chứng**: Serial in ra `[DS18] No sensors found!`

**Nguyên nhân có thể**:
1. Cảm biến chưa kết nối đúng
2. Thiếu điện trở pull-up 4.7kΩ
3. Pin kết nối sai (kiểm tra `DS18_PIN` trong `ConfigStore.h`)
4. Cảm biến bị hỏng

**Giải pháp**:
- Kiểm tra kết nối VCC, GND, DQ
- Đảm bảo có điện trở pull-up 4.7kΩ giữa DQ và VCC
- Thử đổi pin khác
- Kiểm tra bằng multimeter

### Nhiệt độ trả về -127.0°C

**Nguyên nhân**: Cảm biến không kết nối hoặc lỗi đọc

**Giải pháp**:
- Kiểm tra kết nối vật lý
- Kiểm tra `TemperatureService_IsConnected()`
- Tăng delay sau `requestTemperatures()` lên 750-1000ms

### Web UI không cập nhật

**Nguyên nhân**: Auto-refresh interval quá ngắn hoặc lỗi JavaScript

**Giải pháp**:
- Kiểm tra console browser (F12)
- Kiểm tra API `/temperature/info` có hoạt động không
- Tăng interval lên 3-5 giây nếu cần

### Build lỗi

**Lỗi thường gặp**:
- `ESP8266HTTPClient does not name a type`: Đã sửa, dùng `HTTPClient`
- `IPAddress does not name a type`: Đảm bảo include `<ESP8266WiFi.h>`
- Library không tìm thấy: Chạy `pio pkg install`

## 📝 Changelog

### Version 3.0.3
- ✅ Thay thế Relay bằng DS18B20 Temperature Sensor
- ✅ Hỗ trợ ESP8266 (thay vì ESP32)
- ✅ Cảm biến kết nối ở pin D2 (GPIO 4)
- ✅ Web UI hiển thị nhiệt độ real-time
- ✅ API `/temperature/info` và `/temperature/get`
- ✅ Gửi dữ liệu nhiệt độ lên server qua REST API

## 📄 License

[Thêm license của bạn ở đây]

## 👤 Author

[Thêm thông tin tác giả]

## 🙏 Acknowledgments

- DallasTemperature library by Miles Burton
- OneWire library by Paul Stoffregen
- ESP8266 Arduino Core
