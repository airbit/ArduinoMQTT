file(GLOB SRC_FILES
    *.h
    *.cpp)


generate_energia_firmware(mqtt_qos
    SKETCH mqtt_qos.ino
    SRCS ${SRC_FILES})