#include "dta.h"
/*
RS485 TX pin: 32
RS485 RX pin: 35

DI D1 pin: 36
DI D2 pin: 39

*/

#define MODBUS_SLAVE_ID 1
#define MODBUS_BAUD_RATE 19200
#define MODBUS_SERIAL_CONFIG SERIAL_8O1
#define MODBUS_RX_PIN 35
#define MODBUS_TX_PIN 32

// instantiate ModbusMaster object
ModbusMaster node;

DTA dta(MODBUS_SLAVE_ID, MODBUS_BAUD_RATE, MODBUS_SERIAL_CONFIG, MODBUS_RX_PIN, MODBUS_TX_PIN); // Example instantiation with slave ID 1, baud rate 19200, and RX/TX pins 16 and 17

void setup()
{
  // use Serial (port 0); initialize Modbus communication baud rate
  Serial.begin(115200);
  dta.begin(); // Initialize DTA communication
}

void loop()
{
  DTAStatus status = dta.update(); // Update DTA status
  if (status == DTA_OK)
  {
    Serial.println("DTA is ready and working.");
    for (uint8_t i = 0; i < 16; ++i)
    {
      if (dta.checkWorkUnit(i))
      {
        Serial.print("Unit ");
        Serial.print(i);
        Serial.println(" is working.");
        dta.debugCapability(i); // Print capabilities of the unit
        Serial.println("---------------------");
        dta.debugStatus(i); // Print status of the unit
        Serial.println("---------------------");
      }
      else
      {
        Serial.print("Unit ");
        Serial.print(i);
        Serial.println(" is not working or does not exist.");
      }
    }
  }
  else
  {
    Serial.print("DTA status error: ");
    Serial.println(status);
  }
  delay(1000); // Wait for 1 second before the next status check
}
