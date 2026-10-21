#pragma once

// Linux MQTT adapter built on libmosquitto.
// Only compiled when CMake defines BARO_WITH_MOSQUITTO (see CMakeLists.txt),
// so the library still builds without libmosquitto and on Arduino.
#if defined(BARO_WITH_MOSQUITTO) && defined(__linux__) && !defined(ARDUINO)

#include <atomic>
#include <string>
#include "MqttClient.hpp"

struct mosquitto;  // forward declaration, keeps <mosquitto.h> out of this header

namespace baro {

/// MqttClient on top of libmosquitto. Runs its own network thread and
/// reconnects automatically.
///
///   baro::MosquittoClient mqtt("bmp280-pi", "localhost");
///   mqtt.connect();
class MosquittoClient : public MqttClient {
public:
    MosquittoClient(const char* clientId, const char* host, int port = 1883);
    ~MosquittoClient() override;

    MosquittoClient(const MosquittoClient&)            = delete;
    MosquittoClient& operator=(const MosquittoClient&) = delete;

    /// Optional. Call before connect().
    bool setCredentials(const char* user, const char* password);

    /// Starts the background network thread and connects asynchronously.
    /// Returns false only if setup failed; the actual connection may complete
    /// a moment later (check connected()).
    bool connect(int keepAliveSeconds = 60);

    bool connected() override { return connected_.load(); }

    bool publish(const char* topic, const char* payload,
                 bool retain = false, uint8_t qos = 0) override;

private:
    static void onConnect(struct mosquitto*, void* self, int rc);
    static void onDisconnect(struct mosquitto*, void* self, int rc);

    struct mosquitto*  mosq_ = nullptr;
    std::string        host_;
    int                port_;
    std::atomic<bool>  connected_{false};
    bool               started_ = false;
};

} // namespace baro

#endif // BARO_WITH_MOSQUITTO && __linux__ && !ARDUINO
