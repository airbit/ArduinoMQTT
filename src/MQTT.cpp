#include <HardwareSerial.h>
#include "MQTT.h"

namespace MQTT {

    template<typename T>
    T read(uint8_t *buf, size_t &pos);

    template<>
    uint8_t read<uint8_t>(uint8_t *buf, size_t &pos) {
        return buf[pos++];
    }

    template<>
    uint16_t read<uint16_t>(uint8_t *buf, size_t &pos) {
        uint16_t val = buf[pos++] << 8;
        val |= buf[pos++];
        return val;
    }

    template<>
    String read<String>(uint8_t *buf, size_t &pos) {
        uint16_t len = read<uint16_t>(buf, pos);

        String val;
        val.reserve(len);
        for (uint8_t i = 0; i < len; i++)
            val += (char) read<uint8_t>(buf, pos);

        return val;
    }


    BufferedPublish::BufferedPublish(uint8_t flags, uint8_t *data, size_t length) {
        _flags = flags;
        memset(_buffer, 0, MAX_PAYLOAD_BUFFER_ZISE);

        size_t pos = 0;
        _topic = read<String>(data, pos);
        if (qos() > 0)
            _packet_id = read<uint16_t>(data, pos);
        _payload_len = length - pos;
        if (_payload_len > 0) {
            memcpy(_buffer, data + pos, _payload_len);
        }

    }
}