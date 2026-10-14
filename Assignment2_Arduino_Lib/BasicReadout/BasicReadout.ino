#include <Wire.h>
#include "PortableBmp280.h"

baro::WireTransport transport(Wire, baro::Bmp280Driver::kAddressLow);
baro::Bmp280Driver  sensor(transport);

void setup()
{
    Serial.begin(115200);
    Wire.begin();

    if (!sensor.begin()) {
        Serial.println(F("BMP280 not found - check wiring and I2C address (0x76 / 0x77)."));
        while (true) { }
    }

    Serial.print(F("Chip ID: 0x"));
    Serial.println(sensor.chipId(), HEX);   // 0x58 = BMP280, 0x60 = BME280
}

void loop()
{
    baro::Measurement m;

    if (sensor.read(m)) {
        Serial.print(F("Temperature [C]: "));
        Serial.print(m.temperatureC(), 2);
        Serial.print(F("   Pressure [hPa]: "));
        Serial.println(m.pressureHpa(), 2);
    } else {
        Serial.println(F("Read failed"));
    }

    delay(1000);
}
