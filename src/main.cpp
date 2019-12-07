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

String LightSwitchTopic = "/LightSwitch/0/";

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
  for (uint16_t i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    msg += (char)payload[i];
  }
  Serial.println();

  String topicStr = String(topic);

  if(topicStr.startsWith(LightSwitchTopic+"0")||topicStr.startsWith(LightSwitchTopic+"1")){
    
    Serial.println(topicStr.charAt(topicStr.length()-1));
    
    SwitchRelay(topicStr.charAt(topicStr.length()-1)-'0',(char)payload[0]=='1');

  }
  
}

char* str2ch(String command){
    if(command.length()!=0){
        char *p = const_cast<char*>(command.c_str());
        return p;
    }
    return const_cast<char*>("");
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),"test1","test1",str2ch(LightSwitchTopic+"Status"),0,true,str2ch("OFFLINE"))) {
      Serial.println("connected");

      client.subscribe(str2ch(LightSwitchTopic+"#"));
      
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
  //pinMode(SwitchPins[0], INPUT);
  //pinMode(SwitchPins[1], INPUT);
  SwitchRelay(0,false);
  SwitchRelay(1,false);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  while(!client.connected()) {
    reconnect();    
  }
  Serial.println();
  Serial.println("Publishing IP: "+WiFi.localIP().toString());
  client.publish(str2ch(LightSwitchTopic+"IP"),str2ch(WiFi.localIP().toString()));
  
}

void loop() {
  // put your main code here, to run repeatedly:

  if (!client.connected()) {
    reconnect();    
  }
  client.loop();

  if((millis()-mill)>30000){
    client.publish(str2ch(LightSwitchTopic+"Status"),str2ch("ONLINE"));
    mill = millis();
  }

}