#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

uint8_t RelayPin1 = 2;
uint8_t RelayPin2 = 0;
uint8_t SwitchPin1 = 1;
uint8_t SwitchPin2 = 3;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  pinMode(SwitchPin1, INPUT);
  pinMode(SwitchPin2, INPUT);
  
}

void loop() {
  // put your main code here, to run repeatedly:
}