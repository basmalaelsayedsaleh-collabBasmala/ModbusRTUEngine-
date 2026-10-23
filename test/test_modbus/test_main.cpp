#include <Arduino.h>
#include <unity.h>
#include "ModbusRTU.h"

void test_crc16_modbus_standard_vector()
{
const uint8_t testData[] = "123456789";

uint16_t crc = ModbusRTU::calculateCRC(
    testData,
    sizeof(testData) - 1
);

TEST_ASSERT_EQUAL_HEX16(0x4B37, crc);

}

void test_crc_empty_data()
{
const uint8_t testData[] = "";

uint16_t crc = ModbusRTU::calculateCRC(
    testData,
    0
);

TEST_ASSERT_EQUAL_HEX16(0xFFFF, crc);

}

void setup()
{
delay(2000);

UNITY_BEGIN();

RUN_TEST(test_crc16_modbus_standard_vector);
RUN_TEST(test_crc_empty_data);

UNITY_END();

}

void loop()
{
}