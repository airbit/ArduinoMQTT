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

// MQTT_MAX_PACKET_SIZE : Maximum packet size
#ifdef __CC3200R1M1RGC__
#define MQTT_MAX_PACKET_SIZE 1024
#else
#define MQTT_MAX_PACKET_SIZE 128
#endif

#define MQTT_SEND_BLOCK_SIZE 100


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
    uint16_t nextMsgId;
    unsigned long lastOutActivity;
    unsigned long lastInActivity;
    bool pingOutstanding;

    size_t send(uint8_t c);

    size_t send(const uint8_t *buf, size_t len);

    uint16_t readPacket(uint8_t *);

    uint8_t readByte();

    bool write(uint8_t header, uint8_t *buf, uint16_t length);

    uint16_t writeString(String string, uint8_t *buf, uint16_t pos);


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
};


#endif
