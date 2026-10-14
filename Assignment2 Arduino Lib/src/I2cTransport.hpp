#pragma once

#include <stddef.h>
#include <stdint.h>

namespace baro {

/// The only things the sensor driver needs from the outside world.
/// Implement this once per platform (Arduino Wire, Linux /dev/i2c, a test
/// mock, ...). The driver never includes anything MCU-specific.
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
