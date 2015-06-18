/*
 PubSubClient.h - A simple client for MQTT.
  Nicholas O'Leary
  http://knolleary.net
*/

#ifndef PubSubClient_h
#define PubSubClient_h

#include <Arduino.h>
#include <Stream.h>
#include <WiFi/WiFiClient.h>
#include <IPAddress.h>
#include "MQTT.h"

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

struct mqtt_packet_t {
    uint8_t header;
    uint8_t *data;
    size_t length;
    size_t total;
};

class PubSubClient {
public:
    typedef void(*callback_t)(const MQTT::Publish &, void *);

private:

    IPAddress server_ip;
    String server_hostname;
    uint16_t server_port;
    bool _ssl;
    String username, password;
    callback_t _callback;
    void *_callback_data;
    Stream *_stream;

    WiFiClient _client;
    uint8_t buffer[MQTT_MAX_PACKET_SIZE];
    uint16_t keepalive = MQTT_KEEPALIVE;
    uint8_t _max_retries;
    uint16_t nextMsgId;
    unsigned long lastOutActivity;
    unsigned long lastInActivity;
    bool pingOutstanding;

    size_t send(uint8_t c);

    size_t send(const uint8_t *buf, size_t len);

    bool send(MQTT::Message &message);

    bool sendReliably(MQTT::Message &message);

    mqtt_packet_t readPacket();

    MQTT::Message *readMessage();

    uint8_t readByte();

    bool write(uint8_t header, uint8_t *buf, uint16_t length);

    uint16_t writeString(String string, uint8_t *buf, uint16_t pos);

    // Wait for a certain type of packet to come back, optionally check its packet id
    bool wait_for(uint8_t wait_type, uint16_t wait_pid = 0);

    bool processMessage(MQTT::Message *msg, uint8_t match_type = 0, uint16_t match_pid = 0);


public:
    PubSubClient();

    PubSubClient(IPAddress &ip, uint16_t port = 1883, bool ssl = false);

    PubSubClient(String hostname, uint16_t port = 1883, bool ssl = false);

    PubSubClient &set_server(IPAddress &ip, uint16_t port = 1883, bool ssl = false);

    PubSubClient &set_server(String hostname, uint16_t port = 1883, bool ssl = false);

    PubSubClient &unset_server(void);

    PubSubClient &set_auth(String u, String p);

    PubSubClient &unset_auth(void);

    callback_t callback(void) const { return _callback; }

    PubSubClient &set_callback(callback_t cb, void *data = NULL);

    PubSubClient &unset_callback(void);

    // Set the maximum number of retries when waiting for response packets
    PubSubClient &set_max_retries(uint8_t mr) {
        _max_retries = mr;
        return *this;
    }

    Stream *stream(void) const { return _stream; }

    PubSubClient &set_stream(Stream &s);

    PubSubClient &unset_stream(void);

    bool connect(String id);

    bool connect(String id, String willTopic, uint8_t willQos, bool willRetain, String willMessage);

    void disconnect(void);

    bool publish(String topic, String payload);

    bool publish(String topic, const uint8_t *payload, unsigned int plength, bool retained = false);

    bool publish_P(String topic, const uint8_t PROGMEM *payload, unsigned int, bool retained = false);

    bool subscribe(String topic, uint8_t qos = 0);

    bool unsubscribe(String topic);

    bool loop();

    bool connected();

    bool publish(MQTT::Publish &pub);

    // Return the next packet id
    // Needed for constructing our own publish (with QoS>0) or (un)subscribe messages
    uint16_t next_packet_id(void) {
        nextMsgId++;
        if (nextMsgId == 0) nextMsgId = 1;
        return nextMsgId;
    }
};


#endif
