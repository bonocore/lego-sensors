#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

void setup(void) {
  Serial.begin(9600);

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

  delay(1000);
}