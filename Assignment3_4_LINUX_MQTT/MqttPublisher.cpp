#include "MqttPublisher.hpp"

#include <stdio.h>

namespace baro {

namespace {

// Formats value/100 with two decimals using INTEGERS only.
// Why: snprintf("%f") is not supported on AVR, and `int` is 16-bit there,
// so we always go through `long` / `unsigned long` with %ld / %lu.
void formatFixed2(char* out, size_t n, bool negative, unsigned long absValue)
{
    snprintf(out, n, "%s%lu.%02lu",
             negative ? "-" : "",
             absValue / 100UL,
             absValue % 100UL);
}

} // namespace

bool MqttPublisher::send(const char* suffix, const char* payload)
{
    char topic[96];
    const int n = snprintf(topic, sizeof topic, "%s/%s", base_, suffix);
    if (n < 0 || n >= (int)sizeof topic) return false;   // topic too long
    return client_.publish(topic, payload, retain_, 0);
}

bool MqttPublisher::publish(const Measurement& m)
{
    if (!client_.connected()) return false;

    const long temp = (long)m.temperatureCenti;
    char t[16];
    char p[16];
    formatFixed2(t, sizeof t, temp < 0, (unsigned long)(temp < 0 ? -temp : temp));
    formatFixed2(p, sizeof p, false, (unsigned long)m.pressurePa);  // Pa -> hPa

    char json[80];
    snprintf(json, sizeof json,
             "{\"temperature_c\":%s,\"pressure_hpa\":%s}", t, p);

    bool ok = true;
    ok &= send("temperature", t);
    ok &= send("pressure",    p);
    ok &= send("state",       json);
    return ok;
}

} // namespace baro
