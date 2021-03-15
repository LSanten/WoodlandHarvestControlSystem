// By: Odalys Benitez and Leon Santen
// Purpose: Measure voltage of battery system and broadcast

#include <SPI.h>
#include <RH_RF95.h>

#include <Adafruit_INA260.h> //this library requires the BusIO adafruit library to work
#include <LiquidCrystal_I2C.h> //lcd display

// Singleton instance of the radio driver
RH_RF95 rf95;

Adafruit_INA260 ina260 = Adafruit_INA260(); //voltage sensor
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

int led = 8;

void setup() 
{
  pinMode(led, OUTPUT);   
    
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  
  if (!rf95.init())
    Serial.println("init failed"); 

  if (!ina260.begin()) {
    Serial.println("Couldn't find INA260 chip");
    while (1);
  }

  LcdStartup();

}

void loop()
{ 

  float batteryVoltage = measureBatteryVoltage();
  updateLcdDisplay(batteryVoltage);
  
  //LoRaServer_sendTestMessageIfClientSends(); 

  Server_sendBatteryStatus(batteryVoltage);

}


//-----------------------------------------------------------------------------------------------
// Functions for setup code
//-----------------------------------------------------------------------------------------------

void LcdStartup(){
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("LoRa System");
  lcd.setCursor(0,1);
  lcd.print("activated");
  delay(2000);
  lcd.clear();
}

//-----------------------------------------------------------------------------------------------
// Functions for flight code
//-----------------------------------------------------------------------------------------------

float measureBatteryVoltage() {
  //CHECK BATTERY VOLTAGE AND DISPLAY ON LCD
  float batteryVoltage = ina260.readBusVoltage()/1000;
  return batteryVoltage;
}

void updateLcdDisplay(float batteryVoltage){
  lcd.setCursor(0,0);
  lcd.print("Voltage");
  lcd.setCursor(0,1);
  lcd.print(batteryVoltage);
}

void LoRaServer_sendTestMessageIfClientSends(){
  if (rf95.available())
  {
    Serial.println("available:");
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
      digitalWrite(led, HIGH);
//      RH_RF95::printBuffer("request: ", buf, len);
      Serial.print("got request: ");
      Serial.println((char*)buf);
//      Serial.print("RSSI: ");
//      Serial.println(rf95.lastRssi(), DEC);
      
      // Send a reply
      uint8_t data[] = "supppp bitccchhh";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      Serial.println("Sent a reply");
      digitalWrite(led, LOW);
    }
    else
    {
      Serial.println("recv failed");
    }
  }
}

void Server_sendBatteryStatus(float batteryVoltage){
  uint8_t data[] = "24.76 V";
  char v [6];
  dtostrf(batteryVoltage, 6, 2, v); //convert string to char
  //Union v; //declare the union - https://blog.michaelcwright.com/2020/05/19/arduino-float-to-bytes-for-lora-transmission/
  rf95.send(v, sizeof(data));
  rf95.waitPacketSent();
  Serial.println("Sent battery status");
  delay(1000);
}
