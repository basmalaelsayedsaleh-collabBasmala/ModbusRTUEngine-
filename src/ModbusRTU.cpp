#include "ModbusRTU.h"

ModbusRTU::ModbusRTU()
    : _serial(nullptr),
      _slaveId(1),
      _dePin(-1),
      _frameLength(0),
      _lastByteTime(0),
      _frameTimeoutMs(5),
      _discardFrame(false)
{
    for (uint8_t i = 0; i < MAX_REGISTERS; i++)
    {
        _registers[i] = 0;
    }
}

void ModbusRTU::begin(
    HardwareSerial &serial,
    uint32_t baudRate,
    uint8_t slaveId,
    int8_t rxPin,
    int8_t txPin,
    int8_t dePin
)
{
    _serial = &serial;
    _slaveId = slaveId;
    _dePin = dePin;

    _frameLength = 0;
    _discardFrame = false;

    if (baudRate > 19200)
{
    // Modbus RTU uses a fixed 1.75 ms interval
    // for baud rates above 19200.
    _frameTimeoutMs = 2;
}
else if (baudRate > 0)
{
    // Approximate 3.5 character times for 8N1:
    // 10 bits per character.
    uint32_t timeoutUs =
        (35000000UL + baudRate - 1) / baudRate;

    _frameTimeoutMs = (timeoutUs + 999) / 1000;

    if (_frameTimeoutMs == 0)
    {
        _frameTimeoutMs = 1;
    }
}
else
{
    _frameTimeoutMs = 5;
}

    if (_dePin >= 0)
    {
        pinMode(_dePin, OUTPUT);
        digitalWrite(_dePin, LOW);
    }

    _serial->begin(
        baudRate,
        SERIAL_8N1,
        rxPin,
        txPin
    );
}

uint16_t ModbusRTU::calculateCRC(
    const uint8_t *data,
    uint16_t length
)
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

uint16_t ModbusRTU::getRegister(uint8_t address) const
{
    if (address >= MAX_REGISTERS)
    {
        return 0;
    }

    return _registers[address];
}

bool ModbusRTU::setRegister(
    uint8_t address,
    uint16_t value
)
{
    if (address >= MAX_REGISTERS)
    {
        return false;
    }

    _registers[address] = value;

    return true;
}

void ModbusRTU::process()
{
    if (_serial == nullptr)
    {
        return;
    }

    while (_serial->available() > 0)
    {
        int incomingByte = _serial->read();

        if (incomingByte < 0)
        {
            continue;
        }

        _lastByteTime = millis();

        if (_discardFrame)
        {
            continue;
        }

        if (_frameLength < sizeof(_frame))
        {
            _frame[_frameLength++] =
                static_cast<uint8_t>(incomingByte);
        }
        else
        {
            // Discard the oversized frame.
            _frameLength = 0;
            _discardFrame = true;
        }
    }

    if (millis() - _lastByteTime >= _frameTimeoutMs)
    {
        if (_discardFrame)
        {
            _discardFrame = false;
            _frameLength = 0;
        }
        else if (_frameLength > 0)
        {
            processFrame();
            _frameLength = 0;
        }
    }
}

void ModbusRTU::processFrame()
{
    if (_frameLength < 4)
    {
        return;
    }

    uint16_t receivedCRC =
        static_cast<uint16_t>(_frame[_frameLength - 2]) |
        (static_cast<uint16_t>(
            _frame[_frameLength - 1]) << 8);

    uint16_t calculatedCRC =
        calculateCRC(_frame, _frameLength - 2);

    if (receivedCRC != calculatedCRC)
    {
        return;
    }

    uint8_t address = _frame[0];

    // Ignore frames addressed to another slave.
    // Broadcast requests are not implemented.
    if (address != _slaveId)
    {
        return;
    }

    uint8_t functionCode = _frame[1];

    switch (functionCode)
    {
        case 0x03:
        {
            // Read Holding Registers.
            if (_frameLength != 8)
            {
                sendException(functionCode, 0x03);
                return;
            }

            uint16_t startAddress =
                (static_cast<uint16_t>(_frame[2]) << 8) |
                _frame[3];

            uint16_t quantity =
                (static_cast<uint16_t>(_frame[4]) << 8) |
                _frame[5];

            if (quantity == 0 || quantity > 125)
            {
                sendException(functionCode, 0x03);
                return;
            }

            if (startAddress >= MAX_REGISTERS ||
                quantity > MAX_REGISTERS - startAddress)
            {
                sendException(functionCode, 0x02);
                return;
            }

            uint8_t response[255];

            response[0] = _slaveId;
            response[1] = functionCode;
            response[2] =
                static_cast<uint8_t>(quantity * 2);

            uint16_t responseLength = 3;

            for (uint16_t i = 0; i < quantity; i++)
            {
                uint16_t value =
                    _registers[startAddress + i];

                response[responseLength++] =
                    static_cast<uint8_t>(value >> 8);

                response[responseLength++] =
                    static_cast<uint8_t>(value & 0xFF);
            }

            sendCRC(response, responseLength);

            sendResponse(
                response,
                responseLength + 2
            );

            break;
        }

        case 0x06:
        {
            // Write Single Holding Register.
            if (_frameLength != 8)
            {
                sendException(functionCode, 0x03);
                return;
            }

            uint16_t registerAddress =
                (static_cast<uint16_t>(_frame[2]) << 8) |
                _frame[3];

            uint16_t value =
                (static_cast<uint16_t>(_frame[4]) << 8) |
                _frame[5];

            if (registerAddress >= MAX_REGISTERS)
            {
                sendException(functionCode, 0x02);
                return;
            }

            _registers[registerAddress] = value;

            // Function 06 response echoes the request.
            uint8_t response[8];

            for (uint8_t i = 0; i < 6; i++)
            {
                response[i] = _frame[i];
            }

            sendCRC(response, 6);

            sendResponse(response, sizeof(response));

            break;
        }

        default:
        {
            // Illegal Function.
            sendException(functionCode, 0x01);
            break;
        }
    }
}

void ModbusRTU::sendCRC(
    uint8_t *data,
    uint16_t length
)
{
    uint16_t crc = calculateCRC(data, length);

    // Modbus RTU transmits the low CRC byte first.
    data[length] =
        static_cast<uint8_t>(crc & 0xFF);

    data[length + 1] =
        static_cast<uint8_t>(crc >> 8);
}

void ModbusRTU::sendResponse(
    const uint8_t *data,
    uint16_t length
)
{
    if (_serial == nullptr)
    {
        return;
    }

    if (_dePin >= 0)
    {
        digitalWrite(_dePin, HIGH);
        delayMicroseconds(100);
    }

    _serial->write(data, length);
    _serial->flush();

    if (_dePin >= 0)
    {
        digitalWrite(_dePin, LOW);
    }
}

void ModbusRTU::sendException(
    uint8_t functionCode,
    uint8_t exceptionCode
)
{
    uint8_t response[5];

    response[0] = _slaveId;
    response[1] = functionCode | 0x80;
    response[2] = exceptionCode;

    sendCRC(response, 3);

    sendResponse(response, sizeof(response));
}