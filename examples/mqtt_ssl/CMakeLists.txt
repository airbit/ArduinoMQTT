file(GLOB SRC_FILES
    creds.h)

generate_energia_firmware(mqtt_ssl
    SKETCH mqtt_ssl.ino
    SRCS ${SRC_FILES})