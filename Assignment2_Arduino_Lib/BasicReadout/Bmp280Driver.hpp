#pragma once

#include "I2cTransport.hpp"

namespace baro {

/// One compensated reading.
struct Measurement {
    int32_t  temperatureCenti = 0;  ///< 0.01 degC units (2508 = 25.08 degC)
    uint32_t pressurePa       = 0;  ///< Pascal (100653 = 1006.53 hPa)

    float temperatureC() const { return temperatureCenti / 100.0f; }
    float pressureHpa()  const { return pressurePa / 100.0f; }
};

/// Driver for the Bosch BMP280 (and the temperature/pressure part of the BME280 -- humidity is not implemented).
/// All byte transport goes through I2cTransport, so this class compiles
/// unchanged on any platform.
class Bmp280Driver {
public:
    static constexpr uint8_t kAddressLow  = 0x76;  ///< SDO tied to GND
    static constexpr uint8_t kAddressHigh = 0x77;  ///< SDO tied to VDDIO

    /// Does not touch the hardware; call begin() for that.
    explicit Bmp280Driver(I2cTransport& transport) : io_(transport) {}

    Bmp280Driver(const Bmp280Driver&)            = delete;
    Bmp280Driver& operator=(const Bmp280Driver&) = delete;

    /// reset -> check chip ID -> load calibration -> start normal mode
    /// (temp x1, pressure x1). Returns false if any step fails.
    bool begin();

    /// Soft reset (waits for the sensor to restart).
    bool softReset();

    /// Chip ID of the last successful begin() (0x58 = BMP280, 0x60 = BME280).
    uint8_t chipId() const { return chipId_; }

    /// Burst-reads temperature + pressure together and compensates them.
    /// Returns false on bus error or if the sensor has no sample yet.
    bool read(Measurement& out);

private:
    struct Calibration {
        uint16_t T1;
        int16_t  T2, T3;
        uint16_t P1;
        int16_t  P2, P3, P4, P5, P6, P7, P8, P9;
    };

    bool readChipId(uint8_t& id);
    bool loadCalibration();
    bool startNormalMode();

    int32_t  compensateTemperature(int32_t adcT);       // also updates tFine_
    uint32_t compensatePressure(int32_t adcP) const;    // needs tFine_ first

    I2cTransport& io_;
    Calibration   cal_{};
    int32_t       tFine_  = 0;
    uint8_t       chipId_ = 0;
};

} // namespace baro
