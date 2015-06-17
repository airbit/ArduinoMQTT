/*
 Example of using a Stream object to store the message payload

 Uses SRAM library: https://github.com/ennui2342/arduino-sram
 but could use any Stream based class such as SD

  - connects to an MQTT server
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic"
*/

#include "Arduino.h"

#ifdef __CC3200R1M1RGC__

#include "WiFi.h"

#else
#include <ESP8266WiFi.h>
#endif

#include "SRAM.h"
#include <ArduinoMQTT.h>

#include "creds.h"

// Update these with values suitable for your network.
IPAddress server(10, 0, 0, 10);

SRAM sram(4, SRAM_1024);

void callback(const MQTT::Publish& pub, void*pdata) {
  sram.seek(1);

  // do something with the message
  for (uint8_t i = 0; i < pub.payload_len(); i++) {
    sram.write(pub.payload()[i]);
    Serial.write(pub.payload()[i]);
  }

  Serial.println();
}

PubSubClient client(server);

void setup()
{
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
    client.publish("outTopic","hello world");
    client.subscribe("inTopic");
  }

  sram.begin();
  sram.seek(1);
  
  Serial.begin(9600);
}

void loop()
{
  client.loop();
}

