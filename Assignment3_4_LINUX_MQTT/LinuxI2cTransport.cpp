#include "LinuxI2cTransport.hpp"

#if defined(__linux__) && !defined(ARDUINO)

#include <chrono>
#include <thread>
#include <vector>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

namespace baro {

LinuxI2cTransport::~LinuxI2cTransport() { close(); }

bool LinuxI2cTransport::open()
{
    if (fd_ >= 0) return true;

    fd_ = ::open(path_.c_str(), O_RDWR);
    if (fd_ < 0) return false;

    if (::ioctl(fd_, I2C_SLAVE, address_) < 0) {
        close();
        return false;
    }
    return true;
}

void LinuxI2cTransport::close()
{
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

bool LinuxI2cTransport::readRegisters(uint8_t reg, uint8_t* buf, size_t len)
{
    if (fd_ < 0 || len == 0 || len > 0xFFFF) return false;

    // Two messages in ONE transaction: write the register pointer, then
    // read with a repeated START (no STOP in between). Same thing that
    // endTransmission(false) + requestFrom() does on Arduino.
    i2c_msg msgs[2] = {};
    msgs[0].addr  = address_;
    msgs[0].flags = 0;
    msgs[0].len   = 1;
    msgs[0].buf   = &reg;

    msgs[1].addr  = address_;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len   = static_cast<__u16>(len);
    msgs[1].buf   = buf;

    i2c_rdwr_ioctl_data xfer = {};
    xfer.msgs  = msgs;
    xfer.nmsgs = 2;

    return ::ioctl(fd_, I2C_RDWR, &xfer) == 2;
}

bool LinuxI2cTransport::writeRegisters(uint8_t reg, const uint8_t* buf, size_t len)
{
    if (fd_ < 0) return false;

    // One write() = START, addr, reg, data..., STOP.
    std::vector<uint8_t> frame(len + 1);
    frame[0] = reg;
    for (size_t i = 0; i < len; ++i) frame[i + 1] = buf[i];

    const ssize_t n = ::write(fd_, frame.data(), frame.size());
    return n == static_cast<ssize_t>(frame.size());
}

void LinuxI2cTransport::delayMs(uint16_t ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

} // namespace baro

#endif // __linux__ && !ARDUINO
