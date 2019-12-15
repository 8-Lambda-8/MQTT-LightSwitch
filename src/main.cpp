#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

uint8_t RelayPins[] = {2,0};
uint8_t SwitchPins[] = {1,3};
bool LastSwitchState[2];

#include "mqttPwd.h"

#include "wifiPasswd.h"
const char* mqtt_server = "10.0.0.38";

WiFiClient espClient;
PubSubClient client(espClient);

long mill, mqttConnectMillis, wifiConnectMillis;

String LightSwitchTopic = "/LightSwitch/0/";

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  /*while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }*/

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
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),mqtt_user,mqtt_pwd,str2ch(LightSwitchTopic+"Status"),0,true,str2ch("OFFLINE"))) {
      Serial.println("connected");

      client.subscribe(str2ch(LightSwitchTopic+"#"));

      Serial.println("Publishing IP: "+WiFi.localIP().toString());
      client.publish(str2ch(LightSwitchTopic+"IP"),str2ch(WiFi.localIP().toString()),true);
      
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
  Serial.begin(115200);

  pinMode(RelayPins[0], OUTPUT);
  pinMode(RelayPins[1], OUTPUT);
  //pinMode(SwitchPins[0], INPUT);
  //pinMode(SwitchPins[1], INPUT);
  SwitchRelay(0,false);
  SwitchRelay(1,false);

  LastSwitchState[0] = digitalRead(SwitchPins[0]);
  LastSwitchState[1] = digitalRead(SwitchPins[1]);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  reconnect();    
  
}

void loop() {
  // put your main code here, to run repeatedly:

  if (WiFi.status() != WL_CONNECTED && (millis()-wifiConnectMillis)>10000) {
    setup_wifi();
  }

  if (WiFi.status() == WL_CONNECTED && !client.connected() && (millis()-mqttConnectMillis)>5000) {
    reconnect();    
  }

  client.loop();

  if((millis()-mill)>30000){
    client.publish(str2ch(LightSwitchTopic+"Status"),str2ch("ONLINE"),true);
    mill = millis();
  }

  for (uint8_t i = 0; i < 2; i++)
  {
    if(LastSwitchState[i] != digitalRead(SwitchPins[i])){
      
      if(WiFi.status() == WL_CONNECTED && client.connected()){

        if(digitalRead(RelayPins[i]))
        {
          client.publish(str2ch(LightSwitchTopic+i),"1",true);
        }else
        {
          client.publish(str2ch(LightSwitchTopic+i),"0",true);
        }

      }else{//not connected:
        SwitchRelay(i,digitalRead(RelayPins[i]));
      }     


      LastSwitchState[i] = !LastSwitchState[i];
    }
  }
  
}