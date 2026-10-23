def calculate_crc16(data):
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc


def add_crc(data):
    crc = calculate_crc16(data)
    return data + bytes([crc & 0xFF, (crc >> 8) & 0xFF])


def check_crc(frame):
    if len(frame) < 3:
        return False
    expected = calculate_crc16(frame[:-2])
    received = frame[-2] | (frame[-1] << 8)
    return expected == received


def test_crc():
    assert calculate_crc16(b"123456789") == 0x4B37
    print("PASS: CRC-16/MODBUS")


def test_read_holding_registers_request():
    request = add_crc(bytes([1, 3, 0, 0, 0, 2]))
    assert len(request) == 8
    assert check_crc(request)
    assert request[0] == 1
    assert request[1] == 3
    print("PASS: Read Holding Registers request frame")


def test_write_single_register_request():
    request = add_crc(bytes([1, 6, 0, 1, 0, 100]))
    assert len(request) == 8
    assert check_crc(request)
    assert request[0] == 1
    assert request[1] == 6
    print("PASS: Write Single Register request frame")


def test_invalid_crc():
    request = bytearray(add_crc(bytes([1, 3, 0, 0, 0, 2])))
    request[-1] ^= 0xFF
    assert not check_crc(request)
    print("PASS: Invalid CRC rejected by test validator")


if __name__ == "__main__":
    test_crc()
    test_read_holding_registers_request()
    test_write_single_register_request()
    test_invalid_crc()
    print("\nAll Python protocol-frame tests passed.")