//Pins for RFID
#define SS_PIN 2  //D4 (SDA pin) - GPIO2
#define RST_PIN 0 //D3 - GPIO0

//libraries for RFID
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

//Libraries for GYRO
#include <SPI.h>
#include <MFRC522.h>


Adafruit_MPU6050 mpu;

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance.

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

  Serial.println("Initialisation completed");
  delay(100);
}

void loop() {

  //RFID loop
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) 
    {
      String content= "RFID|";
      byte letter;
      for (byte i = 0; i < mfrc522.uid.size; i++) 
      {
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      content.toUpperCase();
      Serial.println(content);
    }
  //GYRO loop
  if(mpu.getMotionInterruptStatus()) {
    /* Get new sensor events with the readings */
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    Serial.print("AccelX");
    Serial.print(a.acceleration.x);
    Serial.print("|AccelY");
    Serial.print(a.acceleration.y);
    Serial.print("|AccelZ");
    Serial.print(a.acceleration.z);
    Serial.print("|GyroX");
    Serial.print(g.gyro.x);
    Serial.print("|GyroY");
    Serial.print(g.gyro.y);
    Serial.print("|GyroZ");
    Serial.print(g.gyro.z);
    Serial.println("");
  }

  delay(500);
}