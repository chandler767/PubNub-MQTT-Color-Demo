// PubNub MQTT example using ATWINC1500.

#include <SPI.h>
#include <WiFi101.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define WINC_CS 8
#define WINC_IRQ 7
#define WINC_RST 4
#define WINC_EN 2 

// Connection info.
const char* ssid = "ATT-WIFI-665M";
const char* password =  "PubNub2018";
const char* mqttServer = "mqtt.pndsn.com";
const int mqttPort = 1883;
const char* clientID = "pub-c-2884742e-59f2-4ced-b9c5-3536de2768f5/sub-c-963bd4b0-e461-11e8-bf53-0a39541862c0/duck";
const char* channelName = "default";

int rled = 11; // The PWM pins the LED is attached to.
int gled = 10;
int bled = 9;

WiFiClient Client;
PubSubClient client(Client);

const size_t capacity = JSON_OBJECT_SIZE(2) + 40;
DynamicJsonDocument doc(capacity);

void callback(char* topic, byte* payload, unsigned int length) {
  String payload_buff;
  for (int i=0;i<length;i++) {
    payload_buff = payload_buff+String((char)payload[i]);
  }
  DeserializationError error = deserializeJson(doc, payload_buff);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  const char* hex = doc["color"]; 
  long long rgb = strtoll( &hex[1], NULL, 16);
  Serial.println(payload_buff); // Print out messages.
  Serial.println(hex); 
  analogWrite(rled, (rgb >> 16)*.15);
  analogWrite(gled, (rgb >> 8 & 0xFF)*.15);
  analogWrite(bled, (rgb & 0xFF)*.15);
}

long lastReconnectAttempt = 0;

boolean reconnect() {
  if (client.connect(clientID)) {
    client.subscribe(channelName); // Subscribe to channel.
  }
  return client.connected();
}

void setup() {
  pinMode(rled, OUTPUT);
  pinMode(gled, OUTPUT);
  pinMode(bled, OUTPUT);

  WiFi.setPins(8, 7, 4, 2);

  Serial.begin(9600);

  Serial.println("Attempting to connect...");
  if(WiFi.begin(ssid, password) != WL_CONNECTED) { // Connect to WiFi.
      Serial.println("Couldn't connect to WiFi.");
      while(1) delay(100);
  }
  client.setServer(mqttServer, mqttPort); // Connect to PubNub.
  client.setCallback(callback);
  lastReconnectAttempt = 0;
}

void loop() {
  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) { // Try to reconnect.
      lastReconnectAttempt = now;
      if (reconnect()) { // Attempt to reconnect.
        lastReconnectAttempt = 0;
      }
    }
  } else { // Connected.
    client.loop();
   // client.publish(channelName,"Hello world!"); // Publish message.
    delay(1000);
  }
}
