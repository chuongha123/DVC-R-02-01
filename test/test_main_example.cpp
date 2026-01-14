/**
 * Ví dụ cách sử dụng RelayService_Test.cpp
 * 
 * File này là ví dụ cách tích hợp test vào project.
 * Bạn có thể:
 * 1. Copy nội dung vào main.cpp để test
 * 2. Hoặc include và gọi hàm test trong code
 */

#include "../include/ConfigStore.h"
#include "../include/RelayService.h"
#include "RelayService_Test.cpp"

void setup() {
    Serial.begin(115200);
    delay(2000); // Đợi Serial monitor sẵn sàng
    
    Serial.println("Starting Relay Service Test...");
    
    // Khởi tạo ConfigStore (cần thiết cho RELAY_PINS và NUM_RELAYS)
    ConfigStore_Init();
    
    // Chạy test đầy đủ
    testRelayService();
    
    // Hoặc chạy test cơ bản (nhanh hơn)
    // testRelayServiceBasic();
}

void loop() {
    // Test sẽ chạy một lần trong setup()
    // Nếu muốn test lặp lại, có thể gọi testRelayServiceBasic() ở đây
    delay(10000);
    
    // Uncomment dòng dưới để test lặp lại mỗi 10 giây
    // testRelayServiceBasic();
}
