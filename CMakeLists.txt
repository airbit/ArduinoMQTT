set(CMAKE_TOOLCHAIN_FILE ~/Dev/github/energia-cmake/cmake/EnergiaToolchain.cmake)

cmake_minimum_required(VERSION 3.1)
project(ArduinoMQTT)

set(CMAKE_VERBOSE_MAKEFILE on)

set(ENERGIA_DEFAULT_BOARD airbit_cc3200)
#set(ENERGIA_DEFAULT_PORT /dev/tty.usbserial-cc3101B)
set(ENERGIA_DEFAULT_PORT /dev/tty.usbserial-AI02S970)

file(GLOB INC_FILES
    src/*.h
    *.h)

file(GLOB SRC_FILES
    src/*.cpp
    *.cpp)

generate_energia_library(ArduinoMQTT
    HDRS ${INC_FILES}
    SRCS ${SRC_FILES})

add_subdirectory(examples/mqtt_auth)
add_subdirectory(examples/mqtt_basic)
add_subdirectory(examples/mqtt_publish_in_callback)
add_subdirectory(examples/mqtt_qos)
#add_subdirectory(examples/mqtt_stream)
add_subdirectory(examples/mqtt_subscriber)
