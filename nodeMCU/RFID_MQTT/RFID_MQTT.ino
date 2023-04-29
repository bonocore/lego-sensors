#define SS_PIN 2  //D4 (SDA pin) - GPIO2
#define RST_PIN 0 //D3 - GPIO0

#include <SPI.h>
#include <MFRC522.h>

#include <ESP8266WiFi.h>
#include <AsyncMqttClient.h>
#include <Ticker.h>

#include "RFID_MQTT_Configuration.h"

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

unsigned long previousMillis = 0; 
const long interval = 10000;
bool carIsCharging = false;
bool messageSent = false;

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}


void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup() 
{
  Serial.println("Setting up...");  
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("MFRC522 Initialised correctly");  
 
 
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials(MQTT_USER,MQTT_PASSWORD);
  connectToWifi();
}
void loop() 
{
    delay(1000);  
    if (!mfrc522.PICC_IsNewCardPresent() && !mfrc522.PICC_IsNewCardPresent()) {  //workaround for getting RFID card present correctly
      //Serial.println("No car detected");
      carIsCharging=false;
      messageSent=false;
      return;
    } else {
        //Serial.println("Car detected");
        carIsCharging=true;
    }
  if(!carIsCharging)
  {
    //Serial.println("No car is charging");  
    return;
  }else
  {
      if(messageSent)
      {
        //Serial.println("Message already sent");  
        return;
      }
      //Serial.println("A car is charging. Sending message");  
      String content= "RFID|";
      for (byte i = 0; i < mfrc522.uid.size; i++) 
      {
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      content.toUpperCase();
      //Serial.println(content);
      //Serial.println("mqtt sent");  
      mqttClient.publish(MQTT_PUB_RFID, 1, true, content.c_str());
      messageSent=true;
  }  
  
} 