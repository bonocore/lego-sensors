void setup() {
  Serial.begin(9600);     // Initialize the Serial interface with baud rate of 9600
}

void loop() {
    if(Serial.available()>0)    //Checks is there any data in buffer 
  {
    Serial.print("Received: ");
    Serial.print(Serial.readString());  //Read serial data send back to serial monitor
  }
  else
  {
    Serial.println("Ping"); //Print ping every 10 secs
    delay(3
    0000);                      // Wait for 10 secs
  }
}
