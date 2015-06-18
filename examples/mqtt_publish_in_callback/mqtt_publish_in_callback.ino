/*
 Publishing in the callback 
 
  - connects to an MQTT server
  - subscribes to the topic "inTopic"
  - when a message is received, republishes it to "outTopic"
  
  This example shows how to publish messages within the
  callback function. The callback function header needs to
  be declared before the PubSubClient constructor and the 
  actual callback defined afterwards.
  This ensures the client reference in the callback function
  is valid.
  
*/

#include "Arduino.h"

#ifdef __CC3200R1M1RGC__

#include "WiFi.h"

#else
#include <ESP8266WiFi.h>
#endif

#include <ArduinoMQTT.h>

#include "creds.h"

// Update these with values suitable for your network.
IPAddress server(10, 0, 0, 10);

PubSubClient client(server);

// Callback function
void callback(const MQTT::Publish &pub, void *pdata) {
    // In order to republish this payload, a copy must be made
    // as the orignal payload buffer will be overwritten whilst
    // constructing the PUBLISH packet.

//    client.publish("outTopic", pub.payload(), pub.payload_len());
    // Copy the payload to a new message
    MQTT::BufferedPublish newpub("outTopic", pub.payload(), pub.payload_len());
    client.publish(newpub);
}

void setup() {
    // Setup console
    Serial.begin(115200);
    delay(10);
    Serial.println();
    Serial.println();

    client.set_callback(callback);

    WiFi.begin(ssid, pass);

    int retries = 0;
    while ((WiFi.status() != WL_CONNECTED) && (retries < 10)) {
        retries++;
        delay(500);
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected");
    }

    if (client.connect("arduinoClient")) {
        client.publish("outTopic", "hello world");
        client.subscribe("inTopic");
    }
}

void loop() {
    client.loop();
}

