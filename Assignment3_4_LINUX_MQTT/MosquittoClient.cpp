#include "MosquittoClient.hpp"

#if defined(BARO_WITH_MOSQUITTO) && defined(__linux__) && !defined(ARDUINO)

#include <string.h>
#include <mosquitto.h>

namespace baro {

MosquittoClient::MosquittoClient(const char* clientId, const char* host, int port)
    : host_(host), port_(port)
{
    mosquitto_lib_init();
    mosq_ = mosquitto_new(clientId, /*clean_session=*/true, /*userdata=*/this);
    if (mosq_) {
        mosquitto_connect_callback_set(mosq_, &MosquittoClient::onConnect);
        mosquitto_disconnect_callback_set(mosq_, &MosquittoClient::onDisconnect);
        // After a drop: retry after 1 s, doubling up to 30 s.
        mosquitto_reconnect_delay_set(mosq_, 1, 30, true);
    }
}

MosquittoClient::~MosquittoClient()
{
    if (mosq_) {
        if (started_) {
            mosquitto_disconnect(mosq_);
            mosquitto_loop_stop(mosq_, false);
        }
        mosquitto_destroy(mosq_);
    }
    mosquitto_lib_cleanup();
}

bool MosquittoClient::setCredentials(const char* user, const char* password)
{
    return mosq_ && mosquitto_username_pw_set(mosq_, user, password) == MOSQ_ERR_SUCCESS;
}

bool MosquittoClient::connect(int keepAliveSeconds)
{
    if (!mosq_) return false;
    if (mosquitto_connect_async(mosq_, host_.c_str(), port_, keepAliveSeconds) != MOSQ_ERR_SUCCESS)
        return false;
    if (mosquitto_loop_start(mosq_) != MOSQ_ERR_SUCCESS)
        return false;
    started_ = true;
    return true;
}

bool MosquittoClient::publish(const char* topic, const char* payload,
                              bool retain, uint8_t qos)
{
    if (!mosq_) return false;
    return mosquitto_publish(mosq_, nullptr, topic,
                             (int)strlen(payload), payload,
                             qos, retain) == MOSQ_ERR_SUCCESS;
}

void MosquittoClient::onConnect(struct mosquitto*, void* self, int rc)
{
    static_cast<MosquittoClient*>(self)->connected_ = (rc == 0);
}

void MosquittoClient::onDisconnect(struct mosquitto*, void* self, int)
{
    static_cast<MosquittoClient*>(self)->connected_ = false;
}

} // namespace baro

#endif // BARO_WITH_MOSQUITTO && __linux__ && !ARDUINO
