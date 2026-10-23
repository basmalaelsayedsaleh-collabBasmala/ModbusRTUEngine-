def calculate_crc16(data):
    crc = 0xFFFF

    for byte in data:
        crc ^= byte

        for _ in range(8):
            if crc & 0x0001:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1

    return crc


test_data = b"123456789"
crc = calculate_crc16(test_data)

print(f"CRC-16/MODBUS = 0x{crc:04X}")

assert crc == 0x4B37, "CRC test failed!"
print("CRC test passed!")