// Host-side check (no Arduino needed): fake sensor returns the Bosch
// datasheet example values, so we know what the answer must be.
#include <cstdio>
#include <cstring>
#include "../src/Bmp280Driver.hpp"

using namespace baro;

struct MockTransport : I2cTransport {
    uint8_t regs[256] = {};
    int delays = 0;

    MockTransport() {
        regs[0xD0] = 0x58;
        // dig_T1..dig_P9 from the Bosch datasheet example (note the negatives)
        const int32_t c[12] = {27504, 26435, -1000, 36477, -10685, 3024,
                               2855, 140, -7, 15500, -14600, 6000};
        for (int i = 0; i < 12; ++i) {
            regs[0x88 + 2*i]     = (uint8_t)(c[i] & 0xFF);
            regs[0x88 + 2*i + 1] = (uint8_t)((c[i] >> 8) & 0xFF);
        }
        const int32_t adcP = 415148, adcT = 519888;
        regs[0xF7] = (adcP >> 12) & 0xFF; regs[0xF8] = (adcP >> 4) & 0xFF; regs[0xF9] = (adcP << 4) & 0xF0;
        regs[0xFA] = (adcT >> 12) & 0xFF; regs[0xFB] = (adcT >> 4) & 0xFF; regs[0xFC] = (adcT << 4) & 0xF0;
    }
    bool readRegisters(uint8_t r, uint8_t* b, size_t n) override { memcpy(b, &regs[r], n); return true; }
    bool writeRegisters(uint8_t r, const uint8_t* b, size_t n) override { memcpy(&regs[r], b, n); return true; }
    void delayMs(uint16_t) override { ++delays; }
};

int main() {
    MockTransport mock;
    Bmp280Driver drv(mock);
    int fail = 0;

    if (!drv.begin()) { puts("FAIL begin"); return 1; }
    printf("chip id      : 0x%02X\n", drv.chipId());
    printf("ctrl_meas    : 0x%02X (expect 0x27)\n", mock.regs[0xF4]);

    Measurement m;
    if (!drv.read(m)) { puts("FAIL read"); return 1; }
    printf("temperature  : %d (x0.01 C)  -> %.2f C   (datasheet: 2508 / 25.08 C)\n", (int)m.temperatureCenti, m.temperatureC());
    printf("pressure     : %lu Pa -> %.2f hPa  (Bosch 64-bit ref: 100653.25 Pa; int32 formula is ~3 Pa coarser)\n", (unsigned long)m.pressurePa, m.pressureHpa());

    if (mock.regs[0xF4] != 0x27) fail = 1;
    if (m.temperatureCenti != 2508) fail = 1;
    if (m.pressurePa < 100648 || m.pressurePa > 100658) fail = 1;

    // "not ready" case: raw value 0x80000 must be rejected
    mock.regs[0xF7] = 0x80; mock.regs[0xF8] = 0; mock.regs[0xF9] = 0;
    if (drv.read(m)) { puts("FAIL: skipped value accepted"); fail = 1; }

    // wrong chip must be rejected
    MockTransport bad; bad.regs[0xD0] = 0x42;
    Bmp280Driver d2(bad);
    if (d2.begin()) { puts("FAIL: bad chip id accepted"); fail = 1; }

    puts(fail ? "RESULT: FAIL" : "RESULT: PASS");
    return fail;
}
