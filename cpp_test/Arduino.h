#ifndef TEST_ARDUINO_H
#define TEST_ARDUINO_H

#include <cstdint>
#include <cstddef>
#include <vector>

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;

extern uint32_t testMillis;

class HardwareSerial
{
public:
std::vector<uint8_t> rx;
std::vector<uint8_t> tx;

void begin(uint32_t, int, int, int) {}

int available()
{
    return static_cast<int>(rx.size());
}

int read()
{
    if (rx.empty())
    {
        return -1;
    }

    uint8_t value = rx.front();
    rx.erase(rx.begin());
    return value;
}

size_t write(const uint8_t *data, size_t length)
{
    tx.insert(tx.end(), data, data + length);
    return length;
}

void flush() {}

void inject(const uint8_t *data, size_t length)
{
    rx.insert(rx.end(), data, data + length);
}

};

#define SERIAL_8N1 0
#define OUTPUT 1
#define HIGH 1
#define LOW 0

inline void pinMode(int, int) {}
inline void digitalWrite(int, int) {}
inline void delayMicroseconds(unsigned int) {}
inline void delay(unsigned int) {}

inline uint32_t millis()
{
return testMillis;
}

#endif