//Pins for RFID
#define SS_PIN 2  //D4 (SDA pin) - GPIO2
#define RST_PIN 0 //D3 - GPIO0


#define WIFI_SSID "GB_PVT_AP"
#define WIFI_PASSWORD "salesforce2023WTl3g0!"

#define MQTT_HOST IPAddress(192, 168, 69, 1)
#define MQTT_PORT 1883
#define MQTT_USER "mqtt"
#define MQTT_PASSWORD "salesforce1"

#define MQTT_PUB_RFID "sensors/rfid"
#define MQTT_PUB_GYRO "sensors/gyro"

//libraries for RFID
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

//Libraries for GYRO
#include <SPI.h>
#include <MFRC522.h>

#include <ESP8266WiFi.h>
#include <AsyncMqttClient.h>
#include <Ticker.h>

Adafruit_MPU6050 mpu;

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance.

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

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

void setup(void) {
  Serial.begin(9600);

  //Initialise RFID
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("MFRC522 Initialised correctly");  

  //Initialise GYRO
  if (!mpu.begin()) {
    Serial.println("Cannot connect to MPU6050!");
    while (1) {
      delay(10);
    }
  }
  Serial.println("Connected to MPU6050");

  //setupt motion detection
  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
  mpu.setMotionDetectionThreshold(1);
  mpu.setMotionDetectionDuration(20);
  mpu.setInterruptPinLatch(true);	// Keep it latched.  Will turn off when reinitialized.
  mpu.setInterruptPinPolarity(true);
  mpu.setMotionInterrupt(true);
 
  //setup mqtt handler
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials(MQTT_USER,MQTT_PASSWORD);  

  //setup wifi
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  connectToWifi();

  Serial.println("Initialisation completed");
  delay(100);
}

void loop() {

  //RFID loop
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) 
    {
      String contentRFID= "RFID|";
      byte letter;
      for (byte i = 0; i < mfrc522.uid.size; i++) 
      {
        contentRFID.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      contentRFID.toUpperCase();
      Serial.println(contentRFID);
      uint16_t packetIdPubRfid = mqttClient.publish(MQTT_PUB_RFID, 1, true, contentRFID.c_str());
      Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_RFID, packetIdPubRfid);
    }
  //GYRO loop
  if(mpu.getMotionInterruptStatus()) {
    /* Get new sensor events with the readings */
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    String contentGYRO= "GYRO|";
    contentGYRO.concat("AccelX");
    contentGYRO.concat(a.acceleration.x);
    contentGYRO.concat("|AccelY");
    contentGYRO.concat(a.acceleration.y);
    contentGYRO.concat("|AccelZ");
    contentGYRO.concat(a.acceleration.z);
    contentGYRO.concat("|GyroX");
    contentGYRO.concat(g.gyro.x);
    contentGYRO.concat("|GyroY");
    contentGYRO.concat(g.gyro.y);
    contentGYRO.concat("|GyroZ");
    contentGYRO.concat(g.gyro.z);
    
    uint16_t packetIdPubGyro = mqttClient.publish(MQTT_PUB_GYRO, 1, true, contentGYRO.c_str());
    Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_GYRO, packetIdPubGyro);
  }

  delay(500);
}