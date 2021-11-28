#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>

uint8_t RelayPins[] = {2, 0};
uint8_t SwitchPins[] = {1, 3};
bool LastSwitchState[sizeof(RelayPins)];

#include "mqttPwd.h"
#include "wifiPasswd.h"

long mill, mqttConnectMillis, wifiConnectMillis;

const String id = "0";
const String LightSwitchTopic = "/LightSwitch/" + id + '/';
const String host = "LightSwitch_" + id;

#include "version.h"
const char* version = __DATE__ " / " __TIME__ " / " __gitVersion__;

WiFiClient espClient;
PubSubClient client(espClient);

void SwitchRelay(uint8_t x, boolean b) {
  Serial.print("switching Relay ");
  Serial.print(x);
  Serial.print(" on Pin ");
  Serial.print(RelayPins[x]);
  Serial.print(" to ");
  Serial.println(b);

  digitalWrite(RelayPins[x], !b);  // invert because Relay is on when on GND
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.printf("\nMessage arrived [%s](L=%d)\n", topic, length);
  String msg = "";
  for (uint16_t i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    msg += (char)payload[i];
  }
  Serial.println();

  String topicStr = String(topic);

  if (topicStr.startsWith(LightSwitchTopic)) {
    topicStr.replace(LightSwitchTopic, "");

    if (topicStr.length() == 1) {
      Serial.println(topicStr.charAt(topicStr.length() - 1));

      SwitchRelay(topicStr.charAt(topicStr.length() - 1) - '0',
                  (char)payload[0] == '1');
    }
  }
}

char* str2ch(String command) {
  if (command.length() != 0) {
    char* p = const_cast<char*>(command.c_str());
    return p;
  }
  return const_cast<char*>("");
}

void setup_wifi() {
  if (WiFi.status() != WL_CONNECTED) {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.setHostname(str2ch(host));
    WiFi.begin(ssid, password);

    delay(500);
    randomSeed(micros());
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  if (WiFi.status() == WL_CONNECTED && !client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = host;
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pwd,
                       str2ch(LightSwitchTopic + "Status"), 0, true,
                       str2ch("OFFLINE"))) {
      Serial.println("connected");

      client.subscribe(str2ch(LightSwitchTopic + "#"));

      Serial.println("Publishing IP: " + WiFi.localIP().toString());
      client.publish(str2ch(LightSwitchTopic + "IP"),
                     str2ch(WiFi.localIP().toString()), true);
      client.publish(str2ch(LightSwitchTopic + "Version"), version, true);

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(74880);
  Serial.println("begin Setup");

  for (uint8_t i = 0; i < sizeof(RelayPins); i++) {
    pinMode(RelayPins[i], OUTPUT);
    pinMode(SwitchPins[i], INPUT);

    SwitchRelay(i, false);
  }

  setup_wifi();  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  ArduinoOTA.setHostname(str2ch(host));
  ArduinoOTA.begin();

  reconnect();
  Serial.println("end Setup");
}

void loop() {
  // put your main code here, to run repeatedly:

  ArduinoOTA.handle();

  if ((millis() - wifiConnectMillis) > 10000) {
    setup_wifi();
    wifiConnectMillis = millis();
  }

  if ((millis() - mqttConnectMillis) > 5000) {
    reconnect();
    mqttConnectMillis = millis();
  }

  client.loop();

  if ((millis() - mill) > 30000) {
    client.publish(str2ch(LightSwitchTopic + "Status"), str2ch("ONLINE"), true);
    mill = millis();
  }

  for (uint8_t i = 0; i < sizeof(RelayPins); i++) {
    if (LastSwitchState[i] != digitalRead(SwitchPins[i])) {
      if (WiFi.status() == WL_CONNECTED && client.connected()) {
        if (digitalRead(RelayPins[i])) {
          client.publish(str2ch(LightSwitchTopic + i), "1", true);
        } else {
          client.publish(str2ch(LightSwitchTopic + i), "0", true);
        }

      } else {  // not connected:
        SwitchRelay(i, digitalRead(RelayPins[i]));
      }

      LastSwitchState[i] = !LastSwitchState[i];
    }
  }
  delay(2);
}