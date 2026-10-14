#include "WireTransport.hpp"

#if defined(ARDUINO)

namespace baro {

bool WireTransport::readRegisters(uint8_t reg, uint8_t* buf, size_t len)
{
    if (len == 0 || len > kMaxRead) return false;

    wire_.beginTransmission(address_);
    wire_.write(reg);
    if (wire_.endTransmission(false) != 0) return false;   // repeated start

    const size_t got = wire_.requestFrom(address_, (uint8_t)len);
    if (got != len) return false;

    for (size_t i = 0; i < len; ++i) {
        buf[i] = (uint8_t)wire_.read();
    }
    return true;
}

bool WireTransport::writeRegisters(uint8_t reg, const uint8_t* buf, size_t len)
{
    if (len > kMaxWrite) return false;

    wire_.beginTransmission(address_);
    wire_.write(reg);
    for (size_t i = 0; i < len; ++i) {
        wire_.write(buf[i]);
    }
    return wire_.endTransmission() == 0;
}

} // namespace baro

#endif // ARDUINO
