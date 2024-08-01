#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

// WiFi credentials
const char ssid[] = "Wokwi-GUEST";
const char password[] = "";

// MQTT Broker
const char mqtt_server[] = "test.mosquitto.org";
const int mqtt_port = 1883;

// MQTT topics for each LED
/* replace the 'replace-text' with some name like " rexday/traffic/green/A;" to make it work and avoid conficts with others */
const char greenTopicA[] = "replace-text/traffic/green/A";
const char yellowTopicA[] = "replace-text/traffic/yellow/A";
const char redTopicA[] = "replace-text/traffic/red/A";
const char greenTopicB[] = "replace-text/traffic/green/B";
const char yellowTopicB[] = "replace-text/traffic/yellow/B";
const char redTopicB[] = "replace-text/traffic/red/B";
const char greenTopicC[] = "replace-text/traffic/green/C";
const char yellowTopicC[] = "replace-text/traffic/yellow/C";
const char redTopicC[] = "replace-text/traffic/red/C";
const char greenTopicD[] = "replace-text/traffic/green/D";
const char yellowTopicD[] = "replace-text/traffic/yellow/D";
const char redTopicD[] = "replace-text/traffic/red/D";
const char stopAllTopic[] = "replace-text/traffic/stopall";

// Define the pins for controlling the traffic lights
const int pinGreenA = 2;
const int pinYellowA = 4;
const int pinRedA = 16;
const int pinGreenB = 17;
const int pinYellowB = 5;
const int pinRedB = 18;
const int pinGreenC = 19;
const int pinYellowC = 21;
const int pinRedC = 22;
const int pinGreenD = 23;
const int pinYellowD = 15;
const int pinRedD = 13;

// Define the durations for each light phase in milliseconds
const unsigned long greenDuration = 10000; // 10 seconds
const unsigned long yellowDuration = 1500; // 1.5 seconds
const unsigned long redDuration = 10000;   // 10 seconds
const unsigned long zebra_crossDuration = 5000; // 5 seconds
// MQTT keep alive interval in seconds
const int keepAliveInterval = 60; // 1 minute

// WiFi client
WiFiClient espClient;

// MQTT client
PubSubClient client(espClient);

// Variable to control stopping all
bool stopAll = false;

// Function prototypes
void setLights(int pinGreen, int pinYellow, int pinRed, bool green, bool yellow, bool red);
void connectToWiFi();
void connectToMQTT();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200); 
  ArduinoOTA.begin(); // Initialize OTA

  // Initialize pins as outputs
  pinMode(pinGreenA, OUTPUT);
  pinMode(pinYellowA, OUTPUT);
  pinMode(pinRedA, OUTPUT);
  pinMode(pinGreenB, OUTPUT);
  pinMode(pinYellowB, OUTPUT);
  pinMode(pinRedB, OUTPUT);
  pinMode(pinGreenC, OUTPUT);
  pinMode(pinYellowC, OUTPUT);
  pinMode(pinRedC, OUTPUT);
  pinMode(pinGreenD, OUTPUT);
  pinMode(pinYellowD, OUTPUT);
  pinMode(pinRedD, OUTPUT);

  // Connect to WiFi
  connectToWiFi();

  // Connect to MQTT
  connectToMQTT();
}

void loop() {
  ArduinoOTA.handle();
  if (!client.connected()) {
    connectToMQTT();
  }
  client.loop();

  // Check if stopping all is requested
  if (stopAll) {
    Serial.println("Received stop all code");
  } else {
    // Phase 1: A and C green, B and D red
    setLights(pinGreenA, pinYellowA, pinRedA, true, false, false);
    setLights(pinGreenB, pinYellowB, pinRedB, false, false, true);
    setLights(pinGreenC, pinYellowC, pinRedC, true, false, false);
    setLights(pinGreenD, pinYellowD, pinRedD, false, false, true);
    client.publish(greenTopicA, "on");
    client.publish(greenTopicB, "off");
    client.publish(greenTopicC, "on");
    client.publish(greenTopicD, "off");

    client.publish(redTopicA, "off");
    client.publish(redTopicB, "on");
    client.publish(redTopicC, "off");
    client.publish(redTopicD, "on");

    client.publish(yellowTopicA, "off");
    client.publish(yellowTopicB, "off");
    client.publish(yellowTopicC, "off");
    client.publish(yellowTopicD, "off");

    Serial.println("1st Phase uploaded to MQTT");
    delay(greenDuration);

    // Phase 2: A and C yellow, B and D red
    setLights(pinGreenA, pinYellowA, pinRedA, false, true, false);
    setLights(pinGreenB, pinYellowB, pinRedB, false, false, true);
    setLights(pinGreenC, pinYellowC, pinRedC, false, true, false);
    setLights(pinGreenD, pinYellowD, pinRedD, false, false, true);
    client.publish(greenTopicA, "off");
    client.publish(greenTopicB, "off");
    client.publish(greenTopicC, "off");
    client.publish(greenTopicD, "off");

    client.publish(redTopicA, "off");
    client.publish(redTopicB, "on");
    client.publish(redTopicC, "off");
    client.publish(redTopicD, "on");

    client.publish(yellowTopicA, "on");
    client.publish(yellowTopicB, "off");
    client.publish(yellowTopicC, "on");
    client.publish(yellowTopicD, "off");

    Serial.println("2nd Phase uploaded to MQTT");

    delay(yellowDuration);

    // Phase 3: A and C red, B and D green
    setLights(pinGreenA, pinYellowA, pinRedA, false, false, true);
    setLights(pinGreenB, pinYellowB, pinRedB, true, false, false);
    setLights(pinGreenC, pinYellowC, pinRedC, false, false, true);
    setLights(pinGreenD, pinYellowD, pinRedD, true, false, false);
    client.publish(greenTopicA, "off");
    client.publish(greenTopicB, "on");
    client.publish(greenTopicC, "off");
    client.publish(greenTopicD, "on");

    client.publish(redTopicA, "on");
    client.publish(redTopicB, "off");
    client.publish(redTopicC, "on");
    client.publish(redTopicD, "off");

    client.publish(yellowTopicA, "off");
    client.publish(yellowTopicB, "off");
    client.publish(yellowTopicC, "off");
    client.publish(yellowTopicD, "off");

    Serial.println("3rd Phase uploaded to MQTT");

    delay(redDuration);

    // Phase 4: A and C red, B and D yellow
    setLights(pinGreenA, pinYellowA, pinRedA, false, false, true);
    setLights(pinGreenB, pinYellowB, pinRedB, false, true, false);
    setLights(pinGreenC, pinYellowC, pinRedC, false, false, true);
    setLights(pinGreenD, pinYellowD, pinRedD, false, true, false);
    client.publish(greenTopicA, "off");
    client.publish(greenTopicB, "off");
    client.publish(greenTopicC, "off");
    client.publish(greenTopicD, "off");

    client.publish(redTopicA, "on");
    client.publish(redTopicB, "off");
    client.publish(redTopicC, "on");
    client.publish(redTopicD, "off");

    client.publish(yellowTopicA, "off");
    client.publish(yellowTopicB, "on");
    client.publish(yellowTopicC, "off");
    client.publish(yellowTopicD, "on");

    Serial.println("4th Phase uploaded to MQTT");

    delay(yellowDuration);

    // Phase 5: All red
    setLights(pinGreenA, pinYellowA, pinRedA, false, false, true);
    setLights(pinGreenB, pinYellowB, pinRedB, false, false, true);
    setLights(pinGreenC, pinYellowC, pinRedC, false, false, true);
    setLights(pinGreenD, pinYellowD, pinRedD, false, false, true);
    client.publish(greenTopicA, "off");
    client.publish(greenTopicB, "off");
    client.publish(greenTopicC, "off");
    client.publish(greenTopicD, "off");

    client.publish(redTopicA, "on");
    client.publish(redTopicB, "on");
    client.publish(redTopicC, "on");
    client.publish(redTopicD, "on");

    client.publish(yellowTopicA, "off");
    client.publish(yellowTopicB, "off");
    client.publish(yellowTopicC, "off");
    client.publish(yellowTopicD, "off");

    Serial.println("5th Phase uploaded to MQTT");

    delay(zebra_crossDuration);
  }
}

void setLights(int pinGreen, int pinYellow, int pinRed, bool green, bool yellow, bool red) {
  digitalWrite(pinGreen, green);
  digitalWrite(pinYellow, yellow);
  digitalWrite(pinRed, red);
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print(F("Connecting to WiFi"));
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(F("WiFi connected"));
}

void connectToMQTT() {
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.setKeepAlive(keepAliveInterval); // Set MQTT keep-alive interval

  while (!client.connected()) {
    Serial.print(F("Connecting to MQTT..."));
    if (client.connect("espClient")) {
      Serial.println(F("connected"));
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(client.state());
      Serial.println(F(" try again in 5 seconds"));
      delay(5000);
    }
  }

  // Subscribe to the stopAll topic after connecting to MQTT
  client.subscribe(stopAllTopic);
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.println(F("Inside callback..."));
  Serial.print(F("Message arrived on topic ["));
  Serial.print(topic);
  Serial.print(F("] Payload: "));

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Handle LED control messages
  if (strcmp(topic, stopAllTopic) == 0) {
    if (payload[0] == 'on') {
      Serial.println(F("Stop all topic received: ON"));
      stopAll = true;
    } else if (payload[0] == 'off') {
      Serial.println(F("Stop all topic received: OFF"));
      stopAll = false;
    }
  }
}
