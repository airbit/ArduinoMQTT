//
// Created by 陶源 on 15/6/18.
//

#ifndef MQTT_H
#define MQTT_H

#include <stdint.h>
#include <stddef.h>
#include <WString.h>

#define MAX_PAYLOAD_BUFFER_ZISE 8196


// MQTT_KEEPALIVE : keepAlive interval in Seconds
#define MQTT_KEEPALIVE 15

#define MQTTPROTOCOLVERSION 3
#define MQTTCONNECT     1 << 4  // Client request to connect to Server
#define MQTTCONNACK     2 << 4  // Connect Acknowledgment
#define MQTTPUBLISH     3 << 4  // Publish message
#define MQTTPUBACK      4 << 4  // Publish Acknowledgment
#define MQTTPUBREC      5 << 4  // Publish Received (assured delivery part 1)
#define MQTTPUBREL      6 << 4  // Publish Release (assured delivery part 2)
#define MQTTPUBCOMP     7 << 4  // Publish Complete (assured delivery part 3)
#define MQTTSUBSCRIBE   8 << 4  // Client Subscribe request
#define MQTTSUBACK      9 << 4  // Subscribe Acknowledgment
#define MQTTUNSUBSCRIBE 10 << 4 // Client Unsubscribe request
#define MQTTUNSUBACK    11 << 4 // Unsubscribe Acknowledgment
#define MQTTPINGREQ     12 << 4 // PING Request
#define MQTTPINGRESP    13 << 4 // PING Response
#define MQTTDISCONNECT  14 << 4 // Client is Disconnecting
#define MQTTReserved    15 << 4 // Reserved

#define MQTTQOS0        (0 << 1)
#define MQTTQOS1        (1 << 1)
#define MQTTQOS2        (2 << 1)

namespace MQTT {
    class Message {
    protected:
        uint8_t _type, _flags;
        uint16_t _packet_id;    // Not all message types use a packet id, but most do

        Message(uint8_t t, uint8_t f = 0) :
                _type(t), _flags(f),
                _packet_id(0) { }

        Message(uint8_t t, uint16_t pid) :
                _type(t), _flags(0),
                _packet_id(pid) { }

    public:
        // Get the message type
        uint8_t type(void) const { return _type; }

        // Get the packet id
        uint16_t packet_id(void) const { return _packet_id; }
    };

    class Publish : public Message {
    protected:
        String _topic;
        uint8_t *_payload;
        size_t _payload_len;

        virtual void init(String topic, uint8_t *payload, size_t len) {
            _topic = topic;
            _payload = payload;
            _payload_len = len;
        }

    public:
        Publish() :
                Message(MQTTPUBLISH),
                _payload(NULL) { }

        Publish(char *topic, uint8_t *payload, size_t len) :
                Message(MQTTPUBLISH) {
            init(topic, payload, len);
        }

        // Get or set retain flag
        bool retain(void) const { return (bool) (_flags & 0x01); }

        Publish &set_retain(bool r = true) {
            _flags = (uint8_t) ((_flags & ~0x01) | r);
            return *this;
        }

        Publish &unset_retain(void) {
            _flags = (uint8_t) (_flags & ~0x01);
            return *this;
        }

        // Get or set QoS value
        uint8_t qos(void) const { return (uint8_t) ((_flags >> 1) & 0x03); }

        Publish &set_qos(uint8_t q, uint16_t pid = 0) {
            if (q > 2)
                q = 2;

            _flags &= ~0x06;
            if (q) {
                _flags |= q << 1;
                _packet_id = pid;
            }
            return *this;
        }

        Publish &unset_qos(void) {
            _flags &= ~0x06;
            return *this;
        }

        // Get or set dup flag
        bool dup(void) const { return (bool) ((_flags >> 3) & 0x01); }

        Publish &set_dup(bool d = true) {
            _flags = (uint8_t) ((_flags & ~0x08) | (d ? 0x08 : 0));
            return *this;
        }

        Publish &unset_dup(void) {
            _flags = (uint8_t) (_flags & ~0x08);
            return *this;
        }

        virtual char *topic(void) const { return (char *) _topic.c_str(); }

        virtual uint8_t *payload(void) const { return _payload; }

        virtual size_t payload_len(void) const { return _payload_len; }

        virtual char *payload_string(void) const { return (char *) _payload; }
    };

    class BufferedPublish : public Publish {
    protected:
        uint8_t _buffer[MAX_PAYLOAD_BUFFER_ZISE];

        virtual void init(String topic, uint8_t *payload, size_t len) {
            _topic = topic;
            _payload_len = len;

            memcpy(_buffer, payload, len);
            memset(_buffer + len, 0, MAX_PAYLOAD_BUFFER_ZISE - len);
        }

    public:
        BufferedPublish(String topic, uint8_t *payload, size_t len) {
            init(topic, payload, len);
        }

        BufferedPublish(String topic, String &payload) {
            _topic = topic;
            _payload_len = 0;
            memset(_buffer, 0, MAX_PAYLOAD_BUFFER_ZISE);
            if (payload.length() > 0) {
                memcpy(_buffer, payload.c_str(), payload.length());
                _payload_len = payload.length();
            }
        }

        BufferedPublish(String topic, const __FlashStringHelper *payload) {
            _topic = topic;
            _payload_len = strlen_P((PGM_P) payload);
            strncpy((char *) _payload, (PGM_P) payload, _payload_len);
        }

        BufferedPublish(String topic, PGM_P payload, size_t length) {
            _topic = topic;
            _payload_len = length;
            memset(_buffer, 0, MAX_PAYLOAD_BUFFER_ZISE);
            memcpy_P(_buffer, payload, length);
        }

        // Construct from a network buffer
        BufferedPublish(uint8_t flags, uint8_t *data, size_t length);


        virtual uint8_t *payload(void) const { return (uint8_t *) _buffer; }

        virtual char *payload_string(void) const { return (char *) _buffer; }
    };
}

#endif //MQTT_H
