#include <cstdint>
#include <iostream>
#include <cassert>

uint16_t calculateModbusCRC(const uint8_t *data, uint16_t length)
{
uint16_t crc = 0xFFFF;

for (uint16_t pos = 0; pos < length; pos++)
{
    crc ^= data[pos];

    for (uint8_t i = 0; i < 8; i++)
    {
        if ((crc & 0x0001) != 0)
        {
            crc >>= 1;
            crc ^= 0xA001;
        }
        else
        {
            crc >>= 1;
        }
    }
}

return crc;

}

int main()
{
const uint8_t testData[] = "123456789";

uint16_t crc = calculateModbusCRC(testData, 9);

std::cout << "CRC-16/MODBUS = 0x"
          << std::hex << crc << std::endl;

assert(crc == 0x4B37);

std::cout << "PASS: CRC-16/MODBUS test" << std::endl;

return 0;

}