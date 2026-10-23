#include <Arduino.h>
#include "ModbusRTU.h"

ModbusRTU modbus;

void setup()
{
    Serial.begin(115200);

    // Configure these pins for your actual ESP32 setup.
    // Do not assume these are connected to an RS-485 transceiver.
    constexpr int8_t RX_PIN = 16;
    constexpr int8_t TX_PIN = 17;

    modbus.begin(
        Serial2,
        9600,
        1,
        RX_PIN,
        TX_PIN
    );

    // Example register values for initial testing.
    modbus.setRegister(0, 100);
    modbus.setRegister(1, 200);

    Serial.println("Modbus RTU engine initialized.");
}

void loop()
{
    modbus.process();
}