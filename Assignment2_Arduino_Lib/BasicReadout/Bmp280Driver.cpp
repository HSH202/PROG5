#include "Bmp280Driver.hpp"

namespace baro {

namespace {
constexpr uint8_t kRegCalib    = 0x88;  // 24 bytes: dig_T1 .. dig_P9
constexpr uint8_t kRegChipId   = 0xD0;
constexpr uint8_t kRegReset    = 0xE0;
constexpr uint8_t kRegCtrlMeas = 0xF4;
constexpr uint8_t kRegPressMsb = 0xF7;  // F7..FC = press[3] + temp[3]

constexpr uint8_t kResetCmd    = 0xB6;
/ osrs_t = 001 (x1), osrs_p = 001 (x1), mode = 11 (normal)
constexpr uint8_t kCtrlMeasNormalX1 = 0x27;

//Value of a raw reading when that channel is switched off /not ready yet.
constexpr int32_t kRawSkipped = 0x80000;

inline uint16_t u16le(const uint8_t* p) { return (uint16_t)(p[0] | (p[1] << 8)); }
inline int16_t  s16le(const uint8_t* p) { return (int16_t)u16le(p); }

inline bool idIsKnown(uint8_t id)
{
    return id == 0x56 || id == 0x57 || id == 0x58   // BMP280 samples / final
        || id == 0x60;                              // BME280
}
} // namespace

//public ---

bool Bmp280Driver::begin()
{
    if (!softReset())              return false;
    if (!readChipId(chipId_))      return false;
    if (!idIsKnown(chipId_))       return false;
    if (!loadCalibration())        return false;
    if (!startNormalMode())        return false;

    io_.delayMs(10);  // let the first conversion finish (~6.5 ms at x1/x1)
    return true;
}

bool Bmp280Driver::softReset()
{
    const uint8_t cmd = kResetCmd;
    if (!io_.writeRegisters(kRegReset, &cmd, 1)) return false;
    io_.delayMs(5);   // datasheet: start-up time 2 ms
    return true;
}

bool Bmp280Driver::read(Measurement& out)
{
    uint8_t b[6];
    if (!io_.readRegisters(kRegPressMsb, b, sizeof b)) return false;

    // Each 20-bit value is left-aligned in 3 bytes (MSB, LSB, XLSB[7:4]).
    const int32_t adcP = ((int32_t)b[0] << 12) | ((int32_t)b[1] << 4) | (b[2] >> 4);
    const int32_t adcT = ((int32_t)b[3] << 12) | ((int32_t)b[4] << 4) | (b[5] >> 4);

    if (adcP == kRawSkipped || adcT == kRawSkipped) return false;

    // Temperature MUST come first: it produces tFine_ used by pressure.
    out.temperatureCenti = compensateTemperature(adcT);
    out.pressurePa       = compensatePressure(adcP);
    return true;
}

// private ---

bool Bmp280Driver::readChipId(uint8_t& id)
{
    return io_.readRegisters(kRegChipId, &id, 1);
}

bool Bmp280Driver::startNormalMode()
{
    const uint8_t v = kCtrlMeasNormalX1;
    return io_.writeRegisters(kRegCtrlMeas, &v, 1);
}

bool Bmp280Driver::loadCalibration()
{
    uint8_t b[24];
    if (!io_.readRegisters(kRegCalib, b, sizeof b)) return false;

    // T1 and P1 are unsigned, everything else is SIGNED 16-bit.
    cal_.T1 = u16le(&b[0]);
    cal_.T2 = s16le(&b[2]);
    cal_.T3 = s16le(&b[4]);
    cal_.P1 = u16le(&b[6]);
    cal_.P2 = s16le(&b[8]);
    cal_.P3 = s16le(&b[10]);
    cal_.P4 = s16le(&b[12]);
    cal_.P5 = s16le(&b[14]);
    cal_.P6 = s16le(&b[16]);
    cal_.P7 = s16le(&b[18]);
    cal_.P8 = s16le(&b[20]);
    cal_.P9 = s16le(&b[22]);
    return true;
}

// Bosch datasheet, 32-bit integer compensation. Result is in 0.01 degC.
int32_t Bmp280Driver::compensateTemperature(int32_t adcT)
{
    const int32_t v1 = (((adcT >> 3) - ((int32_t)cal_.T1 << 1)) * (int32_t)cal_.T2) >> 11;
    const int32_t d  = (adcT >> 4) - (int32_t)cal_.T1;
    const int32_t v2 = (((d * d) >> 12) * (int32_t)cal_.T3) >> 14;

    tFine_ = v1 + v2;
    return (tFine_ * 5 + 128) >> 8;
}

// Bosch datasheet, 32-bit integer compensation. Result is in Pa.
// Integer maths on purpose: on AVR `double` is only 32-bit, so the
// floating-point reference version loses precision there.
uint32_t Bmp280Driver::compensatePressure(int32_t adcP) const
{
    int32_t v1 = (tFine_ >> 1) - 64000;
    int32_t v2 = (((v1 >> 2) * (v1 >> 2)) >> 11) * (int32_t)cal_.P6;
    v2 = v2 + ((v1 * (int32_t)cal_.P5) << 1);
    v2 = (v2 >> 2) + ((int32_t)cal_.P4 << 16);
    v1 = ((((int32_t)cal_.P3 * (((v1 >> 2) * (v1 >> 2)) >> 13)) >> 3)
          + (((int32_t)cal_.P2 * v1) >> 1)) >> 18;
    v1 = ((32768 + v1) * (int32_t)cal_.P1) >> 15;

    if (v1 == 0) return 0;  // avoid division by zero

    uint32_t p = ((uint32_t)(1048576 - adcP) - (uint32_t)(v2 >> 12)) * 3125u;
    if (p < 0x80000000u) p = (p << 1) / (uint32_t)v1;
    else                 p = (p / (uint32_t)v1) * 2;

    v1 = ((int32_t)cal_.P9 * (int32_t)(((p >> 3) * (p >> 3)) >> 13)) >> 12;
    v2 = ((int32_t)(p >> 2) * (int32_t)cal_.P8) >> 13;
    return (uint32_t)((int32_t)p + ((v1 + v2 + (int32_t)cal_.P7) >> 4));
}

} // namespace baro
