/**
 * RelayService Test Case
 * 
 * File test này kiểm tra các chức năng của RelayService:
 * - Khởi tạo relay
 * - Bật/tắt relay theo index (1, 2, ...)
 * - Bật/tắt relay theo pin number
 * - Đọc trạng thái relay
 * - Test với nhiều relay (mở rộng)
 * - Test edge cases (index ngoài phạm vi)
 * 
 * Cách sử dụng:
 * 1. Copy nội dung hàm testRelayService() vào setup() hoặc loop() trong main.cpp
 * 2. Hoặc include file này và gọi testRelayService() trong code
 */

#include "../include/RelayService.h"
#include "../include/ConfigStore.h"
#include <Arduino.h>

// Biến để theo dõi test
static int testPassed = 0;
static int testFailed = 0;

// Helper function để in kết quả test
void printTestResult(const char* testName, bool passed) {
    if (passed) {
        Serial.print("[PASS] ");
        testPassed++;
    } else {
        Serial.print("[FAIL] ");
        testFailed++;
    }
    Serial.println(testName);
}

// Test 1: Kiểm tra khởi tạo relay
void test_RelayInit() {
    Serial.println("\n=== Test 1: Khởi tạo Relay ===");
    
    RelayService_Begin();
    
    // Kiểm tra tất cả relay đều ở trạng thái OFF sau khi khởi tạo
    bool allOff = true;
    for (int i = 1; i <= NUM_RELAYS; i++) {
        if (RelayService_Get(i) != false) {
            allOff = false;
            Serial.printf("  Relay %d không ở trạng thái OFF sau khởi tạo\n", i);
        }
    }
    
    printTestResult("Tất cả relay ở trạng thái OFF sau khởi tạo", allOff);
    Serial.printf("  Số lượng relay: %d\n", NUM_RELAYS);
    Serial.print("  Các pin relay: ");
    for (int i = 0; i < NUM_RELAYS; i++) {
        Serial.printf("GPIO%d", RELAY_PINS[i]);
        if (i < NUM_RELAYS - 1) Serial.print(", ");
    }
    Serial.println();
}

// Test 2: Bật/tắt relay theo index
void test_RelaySetByIndex() {
    Serial.println("\n=== Test 2: Bật/tắt Relay theo Index ===");
    
    // Test bật từng relay
    for (int i = 1; i <= NUM_RELAYS; i++) {
        RelayService_Set(i, true);
        delay(100);
        bool state = RelayService_Get(i);
        printTestResult(String("Bật relay " + String(i)).c_str(), state == true);
        delay(200);
        
        // Tắt relay
        RelayService_Set(i, false);
        delay(100);
        state = RelayService_Get(i);
        printTestResult(String("Tắt relay " + String(i)).c_str(), state == false);
        delay(200);
    }
}

// Test 3: Bật/tắt relay theo pin number
void test_RelaySetByPin() {
    Serial.println("\n=== Test 3: Bật/tắt Relay theo Pin Number ===");
    
    for (int i = 0; i < NUM_RELAYS; i++) {
        int pin = RELAY_PINS[i];
        
        // Bật relay theo pin
        RelayService_SetByPin(pin, true);
        delay(100);
        bool state = RelayService_Get(i + 1); // Index bắt đầu từ 1
        printTestResult(String("Bật relay qua pin GPIO" + String(pin)).c_str(), state == true);
        delay(200);
        
        // Tắt relay theo pin
        RelayService_SetByPin(pin, false);
        delay(100);
        state = RelayService_Get(i + 1);
        printTestResult(String("Tắt relay qua pin GPIO" + String(pin)).c_str(), state == false);
        delay(200);
    }
}

// Test 4: Test với index ngoài phạm vi (edge cases)
void test_RelayEdgeCases() {
    Serial.println("\n=== Test 4: Edge Cases (Index ngoài phạm vi) ===");
    
    // Test với index = 0 (không hợp lệ)
    RelayService_Set(0, true);
    bool state = RelayService_Get(0);
    printTestResult("Index 0 không hợp lệ (phải trả về false)", state == false);
    
    // Test với index > NUM_RELAYS
    RelayService_Set(NUM_RELAYS + 1, true);
    state = RelayService_Get(NUM_RELAYS + 1);
    printTestResult(String("Index " + String(NUM_RELAYS + 1) + " không hợp lệ (phải trả về false)").c_str(), state == false);
    
    // Test với index âm
    RelayService_Set(-1, true);
    state = RelayService_Get(-1);
    printTestResult("Index âm không hợp lệ (phải trả về false)", state == false);
}

// Test 5: Test tuần tự bật/tắt nhiều relay
void test_RelaySequence() {
    Serial.println("\n=== Test 5: Tuần tự bật/tắt nhiều relay ===");
    
    // Bật tất cả relay
    Serial.println("  Bật tất cả relay...");
    for (int i = 1; i <= NUM_RELAYS; i++) {
        RelayService_Set(i, true);
        delay(100);
        bool state = RelayService_Get(i);
        if (state != true) {
            printTestResult(String("Relay " + String(i) + " không bật được").c_str(), false);
            return;
        }
    }
    printTestResult("Tất cả relay đã bật", true);
    delay(500);
    
    // Tắt tất cả relay
    Serial.println("  Tắt tất cả relay...");
    for (int i = 1; i <= NUM_RELAYS; i++) {
        RelayService_Set(i, false);
        delay(100);
        bool state = RelayService_Get(i);
        if (state != false) {
            printTestResult(String("Relay " + String(i) + " không tắt được").c_str(), false);
            return;
        }
    }
    printTestResult("Tất cả relay đã tắt", true);
}

// Test 6: Test với pin không tồn tại
void test_RelayInvalidPin() {
    Serial.println("\n=== Test 6: Test với Pin không tồn tại ===");
    
    // Test với pin không có trong mảng RELAY_PINS
    int invalidPin = 99; // Pin không tồn tại
    RelayService_SetByPin(invalidPin, true);
    
    // Kiểm tra không có relay nào bị bật
    bool anyOn = false;
    for (int i = 1; i <= NUM_RELAYS; i++) {
        if (RelayService_Get(i) == true) {
            anyOn = true;
            break;
        }
    }
    printTestResult("Pin không tồn tại không làm bật relay nào", !anyOn);
}

// Test 7: Test mở rộng - kiểm tra NUM_RELAYS tự động tính đúng
void test_RelayExpansion() {
    Serial.println("\n=== Test 7: Kiểm tra tính mở rộng ===");
    
    Serial.printf("  Số lượng relay hiện tại: %d\n", NUM_RELAYS);
    Serial.print("  Danh sách pin: ");
    for (int i = 0; i < NUM_RELAYS; i++) {
        Serial.printf("GPIO%d", RELAY_PINS[i]);
        if (i < NUM_RELAYS - 1) Serial.print(", ");
    }
    Serial.println();
    
    // Kiểm tra NUM_RELAYS khớp với số phần tử trong mảng
    int expectedRelays = sizeof(RELAY_PINS) / sizeof(RELAY_PINS[0]);
    printTestResult(String("NUM_RELAYS (" + String(NUM_RELAYS) + ") khớp với kích thước mảng (" + String(expectedRelays) + ")").c_str(), 
                    NUM_RELAYS == expectedRelays);
    
    Serial.println("\n  Để mở rộng thêm relay:");
    Serial.println("  1. Mở file ConfigStore.h");
    Serial.println("  2. Thêm pin vào mảng RELAY_PINS[]");
    Serial.println("  3. Ví dụ: const int RELAY_PINS[] = {15, 2, 4, 5, 16, 17, 18, 19};");
    Serial.println("  4. NUM_RELAYS sẽ tự động cập nhật");
}

// Test 8: Test hiệu năng - bật/tắt nhanh
void test_RelayPerformance() {
    Serial.println("\n=== Test 8: Test hiệu năng (bật/tắt nhanh) ===");
    
    unsigned long startTime = millis();
    int iterations = 50;
    
    for (int i = 0; i < iterations; i++) {
        for (int relay = 1; relay <= NUM_RELAYS; relay++) {
            RelayService_Set(relay, true);
            RelayService_Set(relay, false);
        }
    }
    
    unsigned long endTime = millis();
    unsigned long duration = endTime - startTime;
    float avgTime = (float)duration / (iterations * NUM_RELAYS * 2);
    
    Serial.printf("  Thời gian: %lu ms cho %d lần bật/tắt (%d relay)\n", 
                  duration, iterations * NUM_RELAYS * 2, NUM_RELAYS);
    Serial.printf("  Trung bình: %.2f ms/lần\n", avgTime);
    printTestResult("Test hiệu năng hoàn thành", true);
}

// Hàm chính chạy tất cả test
void testRelayService() {
    Serial.println("\n");
    Serial.println("========================================");
    Serial.println("   RELAY SERVICE TEST SUITE");
    Serial.println("========================================");
    Serial.printf("Số lượng relay: %d\n", NUM_RELAYS);
    Serial.print("Các pin: ");
    for (int i = 0; i < NUM_RELAYS; i++) {
        Serial.printf("GPIO%d", RELAY_PINS[i]);
        if (i < NUM_RELAYS - 1) Serial.print(", ");
    }
    Serial.println("\n");
    
    // Reset counters
    testPassed = 0;
    testFailed = 0;
    
    // Chạy các test
    test_RelayInit();
    delay(500);
    
    test_RelaySetByIndex();
    delay(500);
    
    test_RelaySetByPin();
    delay(500);
    
    test_RelayEdgeCases();
    delay(500);
    
    test_RelaySequence();
    delay(500);
    
    test_RelayInvalidPin();
    delay(500);
    
    test_RelayExpansion();
    delay(500);
    
    test_RelayPerformance();
    
    // Tổng kết
    Serial.println("\n========================================");
    Serial.println("   KẾT QUẢ TEST");
    Serial.println("========================================");
    Serial.printf("PASSED: %d\n", testPassed);
    Serial.printf("FAILED: %d\n", testFailed);
    Serial.printf("TOTAL:  %d\n", testPassed + testFailed);
    Serial.println("========================================\n");
}

// Hàm test đơn giản - chỉ test cơ bản (dùng khi không có phần cứng)
void testRelayServiceBasic() {
    Serial.println("\n=== RELAY SERVICE BASIC TEST ===");
    
    // Khởi tạo
    RelayService_Begin();
    Serial.println("✓ RelayService initialized");
    
    // Test đọc trạng thái
    Serial.println("\nTrạng thái ban đầu:");
    for (int i = 1; i <= NUM_RELAYS; i++) {
        bool state = RelayService_Get(i);
        Serial.printf("  Relay %d (GPIO%d): %s\n", i, RELAY_PINS[i-1], state ? "ON" : "OFF");
    }
    
    // Test bật/tắt
    Serial.println("\nTest bật/tắt:");
    for (int i = 1; i <= NUM_RELAYS; i++) {
        Serial.printf("  Bật relay %d...\n", i);
        RelayService_Set(i, true);
        delay(200);
        Serial.printf("    Trạng thái: %s\n", RelayService_Get(i) ? "ON" : "OFF");
        
        Serial.printf("  Tắt relay %d...\n", i);
        RelayService_Set(i, false);
        delay(200);
        Serial.printf("    Trạng thái: %s\n", RelayService_Get(i) ? "ON" : "OFF");
    }
    
    Serial.println("\n✓ Basic test completed\n");
}
