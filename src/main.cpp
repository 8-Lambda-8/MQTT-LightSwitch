#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

uint8_t RelayPins[] = {2,0};
uint8_t SwitchPins[] = {1,3};


#include "wifiPasswd.h"
const char* ssid;
const char* password;
const char* mqtt_server = "MQTT";

WiFiClient espClient;
PubSubClient client(espClient);

long mill;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void SwitchRelay(uint8_t x,boolean b){

  Serial.print("switching Relay ");
  Serial.print(x);
  Serial.print(" on Pin ");
  Serial.print(RelayPins[x]);
  Serial.print(" to ");
  Serial.println(b);

  digitalWrite(RelayPins[x],!b); //invert because Relay is on when on GND

}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  
  pinMode(RelayPins[0], OUTPUT);
  pinMode(RelayPins[1], OUTPUT);
  pinMode(SwitchPins[0], INPUT);
  pinMode(SwitchPins[1
], INPUT);

  setup_wifi();
  
}

void loop() {
  // put your main code here, to run repeatedly:
}