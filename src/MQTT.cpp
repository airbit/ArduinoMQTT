#include "MQTT.h"

namespace MQTT {
    // First some convenience functions
    void write(uint8_t *buf, size_t &bufpos, uint16_t data) {
        buf[bufpos++] = (uint8_t) (data >> 8);
        buf[bufpos++] = (uint8_t) (data & 0xff);
    }

    void write(uint8_t *buf, size_t &bufpos, uint8_t *data, size_t dlen) {
        memcpy(buf + bufpos, data, dlen);
        bufpos += dlen;
    }

    void write(uint8_t *buf, size_t &bufpos, String str) {
        const char *c = str.c_str();
        size_t length_pos = bufpos;
        bufpos += 2;
        uint16_t count = 0;
        while (*c) {
            buf[bufpos++] = (uint8_t) *c++;
            count++;
        }
        write(buf, length_pos, count);
    }

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



    // Message class
    bool Message::write_fixed_header(uint8_t *buf, size_t &bufpos, size_t rlength) {
        buf[bufpos] = _type << 4;

        switch (_type) {
            case MQTT_PUBLISH:
                buf[bufpos] |= _flags & 0x0f;
                break;
            case MQTT_PUBREL:
            case MQTT_SUBSCRIBE:
            case MQTT_UNSUBSCRIBE:
                buf[bufpos] |= 0x02;
        }
        bufpos++;

        // Remaning length
        do {
            uint8_t digit = (uint8_t) (rlength & 0x7f);
            rlength >>= 7;
            if (rlength)
                digit |= 0x80;
            buf[bufpos++] = digit;
        } while (rlength);

        return true;
    }

    bool Message::write_packet_id(uint8_t *buf, size_t &bufpos) {
        write(buf, bufpos, _packet_id);
    }


    bool Message::send(Stream &stream, size_t block_size) {
        uint8_t packet[MQTT_MAX_PACKET_SIZE];
        return send(stream, packet, block_size);
    }

    bool Message::send(Stream &stream, uint8_t *buffer, size_t block_size) {
        size_t remaining_length = 0;
        write_variable_header(buffer + 5, remaining_length);
        write_payload(buffer + 5, remaining_length);

        uint8_t fixed_header[5];
        size_t fixed_len = 0;
        write_fixed_header(fixed_header, fixed_len, remaining_length);

        uint8_t *real_packet = buffer + 5 - fixed_len;
        size_t real_len = remaining_length + fixed_len;
        memcpy(real_packet, fixed_header, fixed_len);

//        Serial.println("Sending Packet:");
//        for (int i = 0; i < real_len; ++i) {
//            Serial.print(real_packet[i]);
//            Serial.print(" ");
//        }
//        Serial.println();

        // write with block to fix wired issue that client.write in cc3200 can not more than 100 bytes.
        size_t sent = 0;
        size_t count = 0;
        size_t ret = 0;

        if (block_size == 0) {
            block_size = MQTT_SEND_BLOCK_SIZE;
        }
        while (sent < real_len) {
            count = min(real_len - sent, block_size);
            sent += (ret = stream.write(const_cast<const uint8_t *>(real_packet + sent), count));
            if (ret != count) break;
        }

        return (sent == real_len);
    }

    bool Publish::write_variable_header(uint8_t *buf, size_t &bufpos) {
        write(buf, bufpos, _topic);
        if (qos())
            write_packet_id(buf, bufpos);
    }

    bool Publish::write_payload(uint8_t *buf, size_t &bufpos) {
        write(buf, bufpos, payload(), payload_len());
    }

    uint8_t Publish::response_type(void) const {
        switch (qos()) {
            case 0:
                return 0;
            case 1:
                return MQTT_PUBACK;
            case 2:
                return MQTT_PUBREC;
            default:
                return 0;
        }
    }

    BufferedPublish::BufferedPublish(uint8_t flags, uint8_t *data, size_t length) {
        _flags = flags;
        memset(_buffer, 0, MQTT_MAX_PAYLOAD_SIZE);

        size_t pos = 0;
        _topic = read<String>(data, pos);
        if (qos() > 0)
            _packet_id = read<uint16_t>(data, pos);
        _payload_len = length - pos;
        if (_payload_len > 0) {
            memcpy(_buffer, data + pos, _payload_len);
        }

    }

    // PublishAck class
    PublishAck::PublishAck(uint16_t pid) :
            Message(MQTT_PUBACK, pid) { }

    PublishAck::PublishAck(uint8_t *data, size_t length) :
            Message(MQTT_PUBACK) {
        size_t pos = 0;
        _packet_id = read<uint16_t>(data, pos);
    }


    // PublishRec class
    PublishRec::PublishRec(uint16_t pid) :
            Message(MQTT_PUBREC, pid) { }

    PublishRec::PublishRec(uint8_t *data, size_t length) :
            Message(MQTT_PUBREC) {
        size_t pos = 0;
        _packet_id = read<uint16_t>(data, pos);
    }

    bool PublishRec::write_variable_header(uint8_t *buf, size_t &bufpos) {
        write_packet_id(buf, bufpos);
    }


    // PublishRel class
    PublishRel::PublishRel(uint16_t pid) :
            Message(MQTT_PUBREL, pid) { }

    PublishRel::PublishRel(uint8_t *data, size_t length) :
            Message(MQTT_PUBREL) {
        size_t pos = 0;
        _packet_id = read<uint16_t>(data, pos);
    }

    bool PublishRel::write_variable_header(uint8_t *buf, size_t &bufpos) {
        write_packet_id(buf, bufpos);
    }


    // PublishComp class
    PublishComp::PublishComp(uint16_t pid) :
            Message(MQTT_PUBREC, pid) { }

    PublishComp::PublishComp(uint8_t *data, size_t length) :
            Message(MQTT_PUBCOMP) {
        size_t pos = 0;
        _packet_id = read<uint16_t>(data, pos);
    }

    bool PublishComp::write_variable_header(uint8_t *buf, size_t &bufpos) {
        write_packet_id(buf, bufpos);
    }

}