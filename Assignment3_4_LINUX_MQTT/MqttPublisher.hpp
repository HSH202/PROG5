#pragma once

#include "Bmp280Driver.hpp"
#include "MqttClient.hpp"

namespace baro {

/// Turns a Measurement into MQTT messages. Platform independent: it only
/// talks to the MqttClient interface.
///
/// Topics published for baseTopic = "home/livingroom/bmp280":
///   home/livingroom/bmp280/temperature   -> 25.08
///   home/livingroom/bmp280/pressure      -> 1006.53      (hPa)
///   home/livingroom/bmp280/state         -> {"temperature_c":25.08,"pressure_hpa":1006.53}
class MqttPublisher {
public:
    /// `baseTopic` is NOT copied: pass a string literal or keep it alive.
    MqttPublisher(MqttClient& client, const char* baseTopic, bool retain = false)
        : client_(client), base_(baseTopic), retain_(retain) {}

    /// Returns false if the client is disconnected or any publish failed.
    bool publish(const Measurement& m);

private:
    bool send(const char* suffix, const char* payload);

    MqttClient& client_;
    const char* base_;
    bool        retain_;
};

} // namespace baro
