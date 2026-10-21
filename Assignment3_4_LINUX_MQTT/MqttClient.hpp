#pragma once

#include <stddef.h>
#include <stdint.h>

namespace baro {

/// The only things the publisher needs from an MQTT library.
/// Implement once per platform (libmosquitto on Linux, PubSubClient on
/// Arduino, a test mock, ...). Same idea as I2cTransport.
class MqttClient {
public:
    virtual ~MqttClient() = default;

    /// True while the broker connection is up.
    virtual bool connected() = 0;

    /// Publish a NUL-terminated text payload. Returns true if it was accepted.
    virtual bool publish(const char* topic, const char* payload,
                         bool retain = false, uint8_t qos = 0) = 0;
};

} // namespace baro
