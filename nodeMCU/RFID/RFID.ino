#define SS_PIN 2  //D4 (SDA pin) - GPIO2
#define RST_PIN 0 //D3 - GPIO0

#include <SPI.h>
#include <MFRC522.h>

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

void setup() 
{
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("MFRC522 Initialised correctly");  
}
void loop() 
{
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    Serial.println("No card");        
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
      Serial.println("No card readable");              
      return;
  }
  Serial.println("Card detected");        
  String content= "RFID|";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  Serial.println(content);
} 