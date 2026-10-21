#pragma once

// Umbrella header: #include <PortableBmp280.h> in a sketch or a Linux program.
// Every platform-specific header guards itself, so including all of them
// is safe everywhere.
#include "I2cTransport.hpp"
#include "Bmp280Driver.hpp"
#include "WireTransport.hpp"        // Arduino only
#include "LinuxI2cTransport.hpp"    // Linux only
#include "MqttClient.hpp"
#include "MqttPublisher.hpp"
#include "MosquittoClient.hpp"      // Linux + libmosquitto only
