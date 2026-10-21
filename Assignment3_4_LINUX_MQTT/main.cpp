// Reads a BMP280 over /dev/i2c-N and publishes to MQTT every 2 seconds.
//
//   ./bmp280_mqtt [i2c-device] [i2c-address] [mqtt-host] [base-topic]
//   ./bmp280_mqtt /dev/i2c-1 0x76 localhost home/livingroom/bmp280

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <thread>

#include "PortableBmp280.h"

static volatile sig_atomic_t g_running = 1;
static void onSignal(int) { g_running = 0; }

int main(int argc, char** argv)
{
    const char* dev   = argc > 1 ? argv[1] : "/dev/i2c-1";
    const int   addr  = argc > 2 ? (int)strtol(argv[2], nullptr, 0) : baro::Bmp280Driver::kAddressLow;
    const char* host  = argc > 3 ? argv[3] : "localhost";
    const char* topic = argc > 4 ? argv[4] : "home/bmp280";

    signal(SIGINT, onSignal);
    signal(SIGTERM, onSignal);

    baro::LinuxI2cTransport bus(dev, (uint8_t)addr);
    if (!bus.open()) {
        fprintf(stderr, "Cannot open %s (I2C enabled? user in 'i2c' group?)\n", dev);
        return 1;
    }

    baro::Bmp280Driver sensor(bus);
    if (!sensor.begin()) {
        fprintf(stderr, "Sensor init failed (wrong address? wiring?)\n");
        return 1;
    }
    printf("Found chip ID 0x%02X\n", sensor.chipId());

#if defined(BARO_WITH_MOSQUITTO)
    baro::MosquittoClient mqtt("bmp280-linux", host);
    if (!mqtt.connect()) {
        fprintf(stderr, "MQTT setup failed\n");
        return 1;
    }
    baro::MqttPublisher publisher(mqtt, topic);
#else
    (void)host; (void)topic;
#endif

    while (g_running) {
        baro::Measurement m;
        if (sensor.read(m)) {
            printf("%.2f C  %.2f hPa\n", m.temperatureC(), m.pressureHpa());
#if defined(BARO_WITH_MOSQUITTO)
            if (!publisher.publish(m)) fprintf(stderr, "publish failed (broker down?)\n");
#endif
        } else {
            fprintf(stderr, "read failed\n");
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    return 0;
}
