#pragma once

// Arduino-only adapter. Guarded so the rest of the library still builds
// on non-Arduino targets (e.g. a Raspberry Pi CMake build).
#if defined(ARDUINO)

#include <Arduino.h>
#include <Wire.h>
#include "I2cTransport.hpp"

namespace baro {

/// I2cTransport on top of the Arduino Wire library.
/// Call wire.begin() yourself in setup() before using the driver.
class WireTransport : public I2cTransport {
public:
    WireTransport(TwoWire& wire, uint8_t address) : wire_(wire), address_(address) {}

    bool readRegisters(uint8_t reg, uint8_t* buf, size_t len) override;
    bool writeRegisters(uint8_t reg, const uint8_t* buf, size_t len) override;
    void delayMs(uint16_t ms) override { delay(ms); }

private:
    // Wire's internal buffer is 32 bytes on AVR boards (31 usable for a write
    // because the register byte takes one slot). The driver never needs more
    // than 24 bytes in one go.
    static constexpr size_t kMaxRead  = 32;
    static constexpr size_t kMaxWrite = 31;

    TwoWire& wire_;
    uint8_t  address_;
};

} // namespace baro

#endif // ARDUINO
