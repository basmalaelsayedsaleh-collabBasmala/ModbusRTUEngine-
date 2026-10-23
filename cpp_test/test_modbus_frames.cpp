#include <iostream>
#include <cassert>
#include <vector>
#include "ModbusRTU.h"
#include "Arduino.h"

uint32_t testMillis = 0;

void addCRC(std::vector<uint8_t>& frame)
{
    uint16_t crc = ModbusRTU::calculateCRC(
        frame.data(),
        static_cast<uint16_t>(frame.size())
    );

    frame.push_back(crc & 0xFF);
    frame.push_back((crc >> 8) & 0xFF);
}

int main()
{
    HardwareSerial serial;
    ModbusRTU modbus;

    modbus.begin(serial, 9600, 1, 16, 17);

    // Set two test registers.
    modbus.setRegister(0, 100);
    modbus.setRegister(1, 200);

    // Request: read two holding registers starting at address 0.
    std::vector<uint8_t> request = {
        1, 0x03, 0x00, 0x00, 0x00, 0x02
    };

    addCRC(request);
    serial.inject(request.data(), request.size());

    // Receive the request.
    modbus.process();

    // Simulate the end of the frame.
    testMillis += 10;
    modbus.process();

    // Expected response:
    // Slave 1, function 3, 4 data bytes, values 100 and 200, CRC.
    std::vector<uint8_t> expected = {
        1, 0x03, 0x04,
        0x00, 100,
        0x00, 200
    };

    addCRC(expected);

    assert(serial.tx == expected);

std::cout << "PASS: Read Holding Registers (0x03)" << std::endl;
// Test writing a single register (function 0x06).
serial.tx.clear();

std::vector<uint8_t> writeRequest = {
    1, 0x06, 0x00, 0x01, 0x12, 0x34
};

addCRC(writeRequest);
serial.inject(writeRequest.data(), writeRequest.size());

modbus.process();

testMillis += 10;
modbus.process();

assert(modbus.getRegister(1) == 0x1234);
assert(serial.tx == writeRequest);

std::cout << "PASS: Write Single Register (0x06)" << std::endl;

// Test rejection of a request with invalid CRC.
serial.tx.clear();

std::vector<uint8_t> badCRCRequest = {
    1, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00
};

serial.inject(badCRCRequest.data(), badCRCRequest.size());

modbus.process();

testMillis += 10;
modbus.process();

assert(serial.tx.empty());

std::cout << "PASS: Invalid CRC rejected" << std::endl;
// Test an invalid register address.
serial.tx.clear();

std::vector<uint8_t> invalidAddressRequest = {
    1, 0x03, 0x00, 0x20, 0x00, 0x01
};

addCRC(invalidAddressRequest);
serial.inject(
    invalidAddressRequest.data(),
    invalidAddressRequest.size()
);

modbus.process();

testMillis += 10;
modbus.process();

assert(serial.tx.size() == 5);
assert(serial.tx[0] == 1);
assert(serial.tx[1] == 0x83);
assert(serial.tx[2] == 0x02);

std::cout << "PASS: Invalid register address rejected" << std::endl;
// Test an unsupported function code.
serial.tx.clear();

std::vector<uint8_t> unsupportedRequest = {
    1, 0x04, 0x00, 0x00, 0x00, 0x01
};

addCRC(unsupportedRequest);
serial.inject(
    unsupportedRequest.data(),
    unsupportedRequest.size()
);

modbus.process();

testMillis += 10;
modbus.process();

assert(serial.tx.size() == 5);
assert(serial.tx[0] == 1);
assert(serial.tx[1] == 0x84);
assert(serial.tx[2] == 0x01);

std::cout << "PASS: Unsupported function rejected" << std::endl;
// Test an invalid register quantity (zero).
serial.tx.clear();

std::vector<uint8_t> invalidQuantityRequest = {
    1, 0x03, 0x00, 0x00, 0x00, 0x00
};

addCRC(invalidQuantityRequest);
serial.inject(
    invalidQuantityRequest.data(),
    invalidQuantityRequest.size()
);

modbus.process();

testMillis += 10;
modbus.process();

assert(serial.tx.size() == 5);
assert(serial.tx[0] == 1);
assert(serial.tx[1] == 0x83);
assert(serial.tx[2] == 0x03);

std::cout << "PASS: Invalid register quantity rejected" << std::endl;
// Test ignoring a request addressed to another slave.
serial.tx.clear();

std::vector<uint8_t> otherSlaveRequest = {
    2, 0x03, 0x00, 0x00, 0x00, 0x01
};

addCRC(otherSlaveRequest);
serial.inject(
    otherSlaveRequest.data(),
    otherSlaveRequest.size()
);

modbus.process();

testMillis += 10;
modbus.process();

assert(serial.tx.empty());

std::cout << "PASS: Other slave request ignored" << std::endl;
// Test that the response CRC is valid.
serial.tx.clear();

std::vector<uint8_t> crcRequest = {
1, 0x03, 0x00, 0x00, 0x00, 0x01
};

addCRC(crcRequest);
serial.inject(crcRequest.data(), crcRequest.size());

modbus.process();

testMillis += 10;
modbus.process();

assert(serial.tx.size() == 7);

uint16_t receivedResponseCRC =
static_cast<uint16_t>(serial.tx[5]) |
(static_cast<uint16_t>(serial.tx[6]) << 8);

uint16_t calculatedResponseCRC =
ModbusRTU::calculateCRC(serial.tx.data(), 5);

assert(receivedResponseCRC == calculatedResponseCRC);

std::cout << "PASS: Response CRC is valid" << std::endl;

return 0;

}
