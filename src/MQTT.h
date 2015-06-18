//
// Created by 陶源 on 15/6/18.
//

#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>
#include <WString.h>
#include <Client.h>

// MQTT_MAX_PACKET_SIZE : Maximum packet size
#ifdef __CC3200R1M1RGC__
#define MQTT_MAX_PACKET_SIZE 1024
#else
#define MQTT_MAX_PACKET_SIZE 128
#endif

#define MQTT_MAX_PAYLOAD_SIZE 896

#define MQTT_SEND_BLOCK_SIZE 64

// MQTT_KEEPALIVE : keepAlive interval in Seconds
#define MQTT_KEEPALIVE 15

#define MQTT_CONNECT     1  // Client request to connect to Server
#define MQTT_CONNACK     2  // Connect Acknowledgment
#define MQTT_PUBLISH     3  // Publish message
#define MQTT_PUBACK      4  // Publish Acknowledgment
#define MQTT_PUBREC      5  // Publish Received (assured delivery part 1)
#define MQTT_PUBREL      6  // Publish Release (assured delivery part 2)
#define MQTT_PUBCOMP     7  // Publish Complete (assured delivery part 3)
#define MQTT_SUBSCRIBE   8  // Client Subscribe request
#define MQTT_SUBACK      9  // Subscribe Acknowledgment
#define MQTT_UNSUBSCRIBE 10 // Client Unsubscribe request
#define MQTT_UNSUBACK    11 // Unsubscribe Acknowledgment
#define MQTT_PINGREQ     12 // PING Request
#define MQTT_PINGRESP    13 // PING Response
#define MQTT_DISCONNECT  14 // Client is Disconnecting
#define MQTT_Reserved    15 // Reserved

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

        // Write the fixed header to a buffer
        bool write_fixed_header(uint8_t *buf, size_t &bufpos, size_t rlength);

        bool write_packet_id(uint8_t *buf, size_t &bufpos);

        // Abstract methods to be implemented by derived classes
        virtual bool write_variable_header(uint8_t *buf, size_t &bufpos) = 0;

        virtual bool write_payload(uint8_t *buf, size_t &bufpos) { }

    public:

        virtual uint8_t response_type(void) const { return 0; }

        // Send the message out
        bool send(Stream &stream, size_t block_size = MQTT_SEND_BLOCK_SIZE);
        bool send(Stream &stream, uint8_t *buffer, size_t block_size = MQTT_SEND_BLOCK_SIZE);

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

        bool write_variable_header(uint8_t *buf, size_t &bufpos);

        bool write_payload(uint8_t *buf, size_t &bufpos);



        virtual void init(String topic, uint8_t *payload, size_t len) {
            _topic = topic;
            _payload = payload;
            _payload_len = len;
        }

    public:
        Publish() :
                Message(MQTT_PUBLISH),
                _payload(NULL) { }

        Publish(String topic, uint8_t *payload, size_t len) :
                Message(MQTT_PUBLISH) {
            init(topic, payload, len);
        }

        Publish(String topic, const char *payload, size_t len = 0) :
                Message(MQTT_PUBLISH) {
            if (len == 0) {
                len = strlen(payload);
            }
            init(topic, (uint8_t *) payload, len);
        }

        uint8_t response_type(void) const;

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
        uint8_t _buffer[MQTT_MAX_PAYLOAD_SIZE];

        virtual void init(String topic, uint8_t *payload, size_t len) {
            _topic = topic;
            _payload_len = len;

            memcpy(_buffer, payload, len);
            memset(_buffer + len, 0, MQTT_MAX_PAYLOAD_SIZE - len);
        }

    public:
        BufferedPublish(String topic, uint8_t *payload, size_t len) {
            init(topic, payload, len);
        }

        BufferedPublish(String topic, String &payload) {
            _topic = topic;
            _payload_len = 0;
            memset(_buffer, 0, MQTT_MAX_PAYLOAD_SIZE);
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
            memset(_buffer, 0, MQTT_MAX_PAYLOAD_SIZE);
            memcpy_P(_buffer, payload, length);
        }

        // Construct from a network buffer
        BufferedPublish(uint8_t flags, uint8_t *data, size_t length);


        virtual uint8_t *payload(void) const { return (uint8_t *) _buffer; }

        virtual char *payload_string(void) const { return (char *) _buffer; }
    };

    // Response to Publish when qos == 1
    class PublishAck : public Message {
    private:
        bool write_variable_header(uint8_t *buf, size_t &bufpos) { }

    public:
        // Construct with a packet id
        PublishAck(uint16_t pid);

        // Construct from a network buffer
        PublishAck(uint8_t *data, size_t length);
    };


    // First response to Publish when qos == 2
    class PublishRec : public Message {
    private:
        bool write_variable_header(uint8_t *buf, size_t &bufpos);
    public:
        // Construct with a packet id
        PublishRec(uint16_t pid);

        // Construct from a network buffer
        PublishRec(uint8_t *data, size_t length);

        uint8_t response_type(void) const { return MQTT_PUBREL; }

    };


    // Response to PublishRec
    class PublishRel : public Message {
    private:
        bool write_variable_header(uint8_t *buf, size_t &bufpos);

    public:
        // Construct with a packet id
        PublishRel(uint16_t pid);

        // Construct from a network buffer
        PublishRel(uint8_t *data, size_t length);

        uint8_t response_type(void) const { return MQTT_PUBCOMP; }

    };


    // Response to PublishRel
    class PublishComp : public Message {
    private:
        bool write_variable_header(uint8_t *buf, size_t &bufpos);

    public:
        // Construct with a packet id
        PublishComp(uint16_t pid);

        // Construct from a network buffer
        PublishComp(uint8_t *data, size_t length);
    };

    // Ping the broker
    class Ping : public Message {
    private:
        bool write_variable_header(uint8_t *buf, size_t &bufpos) { }
    public:
        // Constructor
        Ping() :
                Message(MQTT_PINGREQ) { }

        // Construct from a network buffer
        Ping(uint8_t *data, size_t length) :
                Message(MQTT_PINGREQ) { }

        uint8_t response_type(void) const { return MQTT_PINGRESP; }

    };


    // Response to Ping
    class PingResp : public Message {
    private:
        bool write_variable_header(uint8_t *buf, size_t &bufpos) { }

    public:
        // Constructor
        PingResp() :
                Message(MQTT_PINGRESP) { }

        // Construct from a network buffer
        PingResp(uint8_t *data, size_t length) :
                Message(MQTT_PINGRESP) { }

    };
}

#endif //MQTT_H
