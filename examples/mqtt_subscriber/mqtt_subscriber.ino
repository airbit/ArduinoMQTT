/*
 MQTT subscriber example

  - connects to an MQTT server
  - subscribes to the topic "inTopic"
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

void callback(const MQTT::Publish& pub) {
  Serial.print(pub.topic());
  Serial.print(" => ");
  Serial.println(pub.payload_string());
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
    client.subscribe("inTopic");
  }
}

void loop()
{
  client.loop();
}
