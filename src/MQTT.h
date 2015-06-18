//
// Created by 陶源 on 15/6/18.
//

#ifndef MQTT_H
#define MQTT_H

#include <stdint.h>
#include <stddef.h>
#include <WString.h>

#define MAX_PAYLOAD_BUFFER_ZISE 8196

namespace MQTT {
    class Publish {
    public:
        virtual char *topic(void) const = 0;
        virtual uint8_t *payload(void) const = 0;
        virtual size_t payload_len(void) const = 0;

        virtual char *payload_string(void) const { return (char *) payload(); }
    };

    class LightPublish: public Publish {
    protected:
        char *_topic;
        uint8_t *_payload;
        size_t _payload_len;

    public:
        LightPublish(char *topic, uint8_t *payload, size_t len):
            _topic(topic),
            _payload(payload),
            _payload_len(len) { }

        virtual char *topic(void) const { return _topic; }

        virtual uint8_t *payload(void) const { return _payload; }

        virtual size_t payload_len(void) const { return _payload_len; }
    };

    class StaticPublish: public Publish {
    protected:
        String _topic;
        uint8_t _payload[MAX_PAYLOAD_BUFFER_ZISE];
        size_t _payload_len;

    public:
        StaticPublish(char *topic, uint8_t *payload, size_t len) {
            _topic = topic;
            _payload_len = len;

            memcpy(_payload, payload, len);
            memset(_payload + len, 0, MAX_PAYLOAD_BUFFER_ZISE - len);
        }

        virtual char *topic(void) const { return (char *)(_topic.c_str()); }

        virtual uint8_t *payload(void) const { return (uint8_t *) _payload; }

        virtual size_t payload_len(void) const { return _payload_len; }
    };
}

#endif //MQTT_H
