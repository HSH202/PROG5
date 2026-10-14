#pragma once

#include <stddef.h>
#include <stdint.h>

namespace baro {
class I2cTransport {
public:
    virtual ~I2cTransport() = default;

    /// Read `len` bytes starting at register `reg`. Returns true on success.
    virtual bool readRegisters(uint8_t reg, uint8_t* buf, size_t len) = 0;

    /// Write `len` bytes starting at register `reg`. Returns true on success.
    virtual bool writeRegisters(uint8_t reg, const uint8_t* buf, size_t len) = 0;

    /// Block for roughly `ms` milliseconds (needed after reset / start-up).
    virtual void delayMs(uint16_t ms) = 0;
};

} // namespace baro
