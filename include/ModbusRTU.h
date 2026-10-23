#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include <Arduino.h>
#include <stdint.h>

class ModbusRTU
{
public:
    static constexpr uint8_t MAX_REGISTERS = 32;

    ModbusRTU();

    void begin(
        HardwareSerial &serial,
        uint32_t baudRate,
        uint8_t slaveId,
        int8_t rxPin,
        int8_t txPin,
        int8_t dePin = -1
    );

    void process();

    uint16_t getRegister(uint8_t address) const;

    bool setRegister(uint8_t address, uint16_t value);

    static uint16_t calculateCRC(
        const uint8_t *data,
        uint16_t length
    );

private:
    HardwareSerial *_serial;

    uint8_t _slaveId;
    int8_t _dePin;

    uint16_t _registers[MAX_REGISTERS];

    uint8_t _frame[256];
    uint16_t _frameLength;

    uint32_t _lastByteTime;
    uint32_t _frameTimeoutMs;
    bool _discardFrame = false;

    void processFrame();

    void sendResponse(
        const uint8_t *data,
        uint16_t length
    );

    void sendException(
        uint8_t functionCode,
        uint8_t exceptionCode
    );

    void sendCRC(
        uint8_t *data,
        uint16_t length
    );
};

#endif