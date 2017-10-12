
// Smart Home Project - Greenhouse Misting Pump Control
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <MicroGear.h>

// Wifi Settings
const char* ssid = "BAZ";
const char* password = "gearman1";

// NETWORK: Static IP details...
IPAddress ip(192, 168, 1, 105);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Netpie Settings
#define APPID       "SKHOME"
#define GEARKEY     "yPUzBTHa5gOeIBs"
#define GEARSECRET  "wwUjTfHDld9eP2U8eXuxAKGUw"
#define ALIAS       "strawberry_watering"

// Relay
int relayPin = D2;

int timer = 0;
WiFiClient client;
MicroGear microgear(client);

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  char strState[msglen];
  for (int i = 0; i < msglen; i++) {
    strState[i] = (char)msg[i];
  }
  String stateStr = String(strState).substring(0,msglen);
  Serial.print(topic);Serial.print("-->");Serial.println(stateStr);
   
  if (String(topic) == "/SKHOME/strawberry_watering"){
     int ontime = stateStr.toInt();  
     digitalWrite(relayPin,HIGH);
     delay(ontime*1000);
     digitalWrite(relayPin,LOW);     
  }
}

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.println("Connected to NETPIE...");
  microgear.setAlias(ALIAS);
  microgear.subscribe("/strawberry_watering");
}

void setup() {
  Serial.begin(9600);

  // NETPIE
  microgear.on(MESSAGE,onMsghandler);
  microgear.on(CONNECTED,onConnected);
 
  WiFi.config(ip, gateway, subnet); // Static IP Setup Info Here...
  if (WiFi.begin(ssid, password)) {
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    
    // OTA
    ArduinoOTA.onStart([]() {
      Serial.println("Start");
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    //uncomment the line below if you want to reset token -->
    microgear.resetToken();
    microgear.init(GEARKEY, GEARSECRET, ALIAS);
    microgear.connect(APPID);
  }

  pinMode(relayPin,OUTPUT);
  digitalWrite(relayPin,LOW);    // relay off
}

void loop() {
  ArduinoOTA.handle();
  if (microgear.connected()) {
      microgear.loop();
      if (timer >= 1000) {
        timer = 0;
      } 
      else {
        timer += 100;
      }
    }
    else {
      Serial.println("connection lost, reconnect...");
      if (timer >= 5000) {
        microgear.connect(APPID);
        timer = 0;
      }
      else {
        timer += 100;
      }
    }
    delay(100);
}
