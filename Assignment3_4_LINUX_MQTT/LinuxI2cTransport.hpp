#pragma once

// Linux-only adapter (Raspberry Pi, Jetson, any board exposing /dev/i2c-N).
// Guarded so the Arduino build ignores this file completely.
#if defined(__linux__) && !defined(ARDUINO)

#include <string>
#include "I2cTransport.hpp"

namespace baro {

/// I2cTransport on top of the Linux i2c-dev interface.
///
///   baro::LinuxI2cTransport bus("/dev/i2c-1", baro::Bmp280Driver::kAddressLow);
///   if (!bus.open()) { /* check permissions / is I2C enabled? */ }
class LinuxI2cTransport : public I2cTransport {
public:
    LinuxI2cTransport(const char* devicePath, uint8_t address)
        : path_(devicePath), address_(address) {}
    ~LinuxI2cTransport() override;

    LinuxI2cTransport(const LinuxI2cTransport&)            = delete;
    LinuxI2cTransport& operator=(const LinuxI2cTransport&) = delete;

    /// Opens the device node and selects the slave address.
    bool open();
    void close();
    bool isOpen() const { return fd_ >= 0; }

    bool readRegisters(uint8_t reg, uint8_t* buf, size_t len) override;
    bool writeRegisters(uint8_t reg, const uint8_t* buf, size_t len) override;
    void delayMs(uint16_t ms) override;

private:
    std::string path_;
    uint8_t     address_;
    int         fd_ = -1;
};

} // namespace baro

#endif // __linux__ && !ARDUINO
