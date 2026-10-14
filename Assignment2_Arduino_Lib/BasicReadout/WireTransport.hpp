#pragma once

#if defined(ARDUINO)

#include <Arduino.h>
#include <Wire.h>
#include "I2cTransport.hpp"

namespace baro {

class WireTransport : public I2cTransport {
public:
    WireTransport(TwoWire& wire, uint8_t address) : wire_(wire), address_(address) {}

    bool readRegisters(uint8_t reg, uint8_t* buf, size_t len) override;
    bool writeRegisters(uint8_t reg, const uint8_t* buf, size_t len) override;
    void delayMs(uint16_t ms) override { delay(ms); }

private:

    static constexpr size_t kMaxRead  = 32;
    static constexpr size_t kMaxWrite = 31;

    TwoWire& wire_;
    uint8_t  address_;
};

} // namespace baro

#endif // ARDUINO
