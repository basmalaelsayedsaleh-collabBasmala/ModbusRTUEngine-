#include <iostream>
#include <cassert>
#include "ModbusRTU.h"

int main()
{
ModbusRTU modbus;

// Test writing and reading a valid register.
assert(modbus.setRegister(0, 1234));
assert(modbus.getRegister(0) == 1234);

std::cout << "PASS: Write and read register" << std::endl;

// Test the last valid register.
assert(modbus.setRegister(31, 5678));
assert(modbus.getRegister(31) == 5678);

std::cout << "PASS: Last valid register" << std::endl;

// Test invalid register addresses.
assert(!modbus.setRegister(32, 9999));
assert(modbus.getRegister(32) == 0);

std::cout << "PASS: Invalid register address" << std::endl;

return 0;

}