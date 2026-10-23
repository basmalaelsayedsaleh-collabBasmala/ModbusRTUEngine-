#include <iostream>
#include <cassert>
#include "ModbusRTU.h"

int main()
{
    const uint8_t testData[] = "123456789";

    uint16_t crc = ModbusRTU::calculateCRC(
        testData,
        sizeof(testData) - 1
    );

    std::cout << "CRC-16/MODBUS = 0x"
              << std::hex << crc << std::endl;

    assert(crc == 0x4B37);

    std::cout << "PASS: Production C++ CRC test" << std::endl;

    return 0;
}
