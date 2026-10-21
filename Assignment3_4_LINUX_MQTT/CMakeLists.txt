cmake_minimum_required(VERSION 3.16)
project(PortableBmp280 CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BARO_WITH_MOSQUITTO "Build the libmosquitto MQTT client" ON)
option(BARO_BUILD_EXAMPLES "Build the Linux example"            ON)

find_package(Threads REQUIRED)

add_library(portable_bmp280
    src/Bmp280Driver.cpp
    src/LinuxI2cTransport.cpp
    src/MqttPublisher.cpp)
target_include_directories(portable_bmp280 PUBLIC src)
target_link_libraries(portable_bmp280 PUBLIC Threads::Threads)

if(BARO_WITH_MOSQUITTO)
    find_library(MOSQUITTO_LIB mosquitto REQUIRED)
    find_path(MOSQUITTO_INC mosquitto.h REQUIRED)
    target_sources(portable_bmp280 PRIVATE src/MosquittoClient.cpp)
    target_include_directories(portable_bmp280 PRIVATE ${MOSQUITTO_INC})
    target_link_libraries(portable_bmp280 PUBLIC ${MOSQUITTO_LIB})
    target_compile_definitions(portable_bmp280 PUBLIC BARO_WITH_MOSQUITTO=1)
endif()

if(BARO_BUILD_EXAMPLES)
    add_executable(bmp280_mqtt examples/linux_mqtt/main.cpp)
    target_link_libraries(bmp280_mqtt PRIVATE portable_bmp280)
endif()
