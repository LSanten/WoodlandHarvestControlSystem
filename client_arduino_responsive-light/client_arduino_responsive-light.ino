// By: Odalys Benitez and Leon Santen
// Purpose: Receive voltage from server and change lights

#include <SPI.h>
#include <RH_RF95.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN    22
#define LED_COUNT  8// How many NeoPixels are attached to the Arduino?
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Singleton instance of the radio driver
RH_RF95 rf95;
float batteryVFloat = 0.01;


void setup() 
{


  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!rf95.init())
    Serial.println("init failed");
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  strip.begin();
  strip.show(); // Initialize all neo pixels to 'off'

}

void loop()
{
  //sendMessageToServer();
  //waitForMessageFromServer(); 
  String batteryVChar = receiveBatteryVoltage();
  //batteryVFloat = atof(batteryVChar); //convert char to float
  batteryVFloat = batteryVChar.toFloat();
  
  Serial.print("Battery Voltage received: ");
  Serial.println(batteryVFloat);

  lightLEDbasedOnVoltage (batteryVFloat);
}

//-----------------------------------------------------------------------------------------------
// Functions for setup code
//-----------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------
// Functions for flight code
//-----------------------------------------------------------------------------------------------

void sendMessageToServer(){
  Serial.println("Sending to rf95_server");
  // Send a message to rf95_server
  uint8_t data[] = "bitch aint a good word";
  rf95.send(data, sizeof(data));
  rf95.waitPacketSent();
}

void waitForMessageFromServer(){
// Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  Serial.println(sizeof(buf));

  if (rf95.waitAvailableTimeout(3000))
  { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len))
   {
      Serial.print("got reply: ");
      Serial.println((char*)buf);
//      Serial.print("RSSI: ");
//      Serial.println(rf95.lastRssi(), DEC);    
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply, is rf95_server running?");
  }
  delay(400);
}

String receiveBatteryVoltage(){
// Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  if (rf95.waitAvailableTimeout(20000))
  { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len))
   {
      Serial.print("got reply: ");
      Serial.println((char*)buf);
//      Serial.print("RSSI: ");
//      Serial.println(rf95.lastRssi(), DEC);    
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply, is rf95_server running?");
  }
  
  delay(400);
  String charvolt = (char*)buf;
  return charvolt;
}

void lightLEDbasedOnVoltage (float batteryVoltage){
  if (batteryVoltage > 26.8){
    // violet
    uint32_t colorStrip = strip.Color(255, 51, 255); // red, green, blue
    strip.fill(colorStrip, 0, 10);
    strip.show();
  }
  else if (batteryVoltage >= 25.0 and batteryVoltage <= 26.8){
    // green
    uint32_t colorStrip = strip.Color(0,255, 0);
    strip.fill(colorStrip, 0, 10);
    strip.show();
  }
  else if (batteryVoltage >= 24.3 and batteryVoltage < 25.0){
    // orange
    uint32_t colorStrip = strip.Color(255, 178, 102);
    strip.fill(colorStrip, 0, 10);
    strip.show();
  }
  else if (batteryVoltage < 24.3){
    // red
    uint32_t colorStrip = strip.Color(255, 0, 0);
    strip.fill(colorStrip, 0, 10);
    strip.show();
  }
}
