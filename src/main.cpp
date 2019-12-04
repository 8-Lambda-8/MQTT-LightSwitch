#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

uint8_t RelayPins[] = {2,0};
uint8_t SwitchPins[] = {1,3};


#include "wifiPasswd.h"
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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String msg = "";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    msg += (char)payload[i];
  }
  Serial.println();

  String topicStr = String(topic);

  if(topicStr.substring(0,15) == "/LightSwitch/0/"){
    
    SwitchRelay(topicStr.charAt(15)-'0',(char)payload[0]=='1');

  }
  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),"test1","test1")) {
      Serial.println("connected");
      
      client.subscribe("/LightSwitch/0/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  
  pinMode(RelayPins[0], OUTPUT);
  pinMode(RelayPins[1], OUTPUT);
  pinMode(SwitchPins[0], INPUT);
  pinMode(SwitchPins[1], INPUT);
  SwitchRelay(0,false);
  SwitchRelay(1,false);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  // put your main code here, to run repeatedly:
}