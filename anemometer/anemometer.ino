

/*******************************************************************************************************************************************************************************************************************************************
 * Title: Anemometer and data logging for classic Vortex anemometer
 * Description: measures wind speed, saves data to SD card
 * Info: watch this video on measuring wind speed if questions: https://www.youtube.com/watch?v=emE6yWWQUHg
 * Created by Leon Santen (leon.santen@icloud.com ), Fall 2020
 *******************************************************************************************************************************************************************************************************************************************
 */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>

// Wiring: SDA pin is connected to A4 and SCL pin to A5.
// Connect to LCD via I2C, default address 0x27 (A0-A2 not jumpered)
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2); // Change to (0x27, 20, 4)) for 20x4 LCD.

const byte LED = 13;
const byte interruptPin = 2;              //declare interrupt pin
const int CSpin = 4;                      //CS pin for SD card (chip select pin)
volatile unsigned long sTime = 0;         //stores start time for wind speed calc
unsigned long dataTimer = 0;              //used to track how often to communicate data
bool flag = false;
volatile bool start = true;         //tracks when a new anemometer measurement starts
volatile float pulseTime = 0;       //stores time between one anemometer realy clsoing and the next
volatile float culPulseTime = 0;    //stores cumulative pusletimes for averaging
volatile unsigned int avgWindCount = 0;     //stores anemometer realy countrs for doing average wind speed
uint32_t dataLoopCount = 0;                //increments every time data is being written and is multiplied with dataLoopLength
String dataString ="";               //hold data to be written to SD card
//File sensorData;                    //create file object for SD card writing
//======================SETTINGS========================
uint32_t dataLoopLength = 10000;    // length of loop to report average speed
//======================SETTINGS========================

void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  pinMode(interruptPin, INPUT);                  //set interrupt pin to input


  //SETUP SD CARD
  Serial.begin(9600);
  Serial.println("Initializing SD card...");
  pinMode(CSpin, OUTPUT);                        //CS pin for SD card writing
  if (!SD.begin(CSpin)) {
    Serial.println("Card failed, or not present.");
    return;
  }
  else{
    Serial.println("card initialized.");
  }
  
   
  dataString = String("Time [s]") + "," + String("Speed [m/s]"); //write CSV (comma seperated vector)
  saveData(dataString);
  Serial.end();

  
  //attachInterrupt(digitalPinToInterrupt(interruptPin), anemometerISR, RISING); //setup interrupt on anemometer input pin, will run anemometerISR function whenever falling edge is detected
  dataTimer = millis();                                 //start data timer

  //LCD startup 
  lcd.init();           //initiate lcd display
  lcd.backlight();
  lcd.setCursor(0, 0); // Set the cursor on the first column and first row.
  lcd.print("   Anemometer   "); // Print the string "Hello World!"
  lcd.setCursor(2, 1); //Set the cursor on the third column and the second row (counting starts at 0!).
  lcd.print("");
  delay(1000);
  lcd.clear();
}
//=====END=====SETUP=====

//==========LOOP=========
void loop() {
  unsigned long rTime = millis();                 //save run-time time stamp  
  unsigned long loopTime = dataLoopCount*dataLoopLength;   //calculate time for next average reading
  
  if((rTime - sTime) > 2500) pulseTime = 0; // if the wind speed has dropped below a certain interval time, set to 0
  
  if (rTime >= loopTime){
    dataLoopCount++; 
    detachInterrupt(interruptPin);          // shut off wind speed measurement when in this loop
    Serial.begin(9600);
    Serial.print("cul Time: ");
    Serial.println(culPulseTime);
    Serial.print("rTime: ");
    Serial.println(rTime);
    Serial.print("loopTime: ");
    Serial.println(loopTime);
    Serial.print("counter: ");
    Serial.println(avgWindCount );
    float aWSpeedMS = getAvgWindSpeedMS(culPulseTime, avgWindCount);
    avgWindCount = 0;     //reset average wind count
    culPulseTime = 0;     //reset cumulative pulse counter
    start = true;
    dataTimer = rTime;     //reset loop timer

    
    Serial.print("avg Wind Speed m/s: ");
    Serial.println(aWSpeedMS);
    
    updateLCD(aWSpeedMS);

    //Write to SD card
    dataString = String(rTime/1000) + "," + String(aWSpeedMS); //write CSV (comma seperated vector)
    saveData(dataString);

    Serial.end(); //end of dataprocessing loop
    }

  if (digitalRead(interruptPin)==LOW and avgWindCount == 0){
    attachInterrupt(digitalPinToInterrupt(interruptPin), anemometerISR, RISING); //turn interrupt back on
  }
    
}//=======LOOP=END======= 

//=====FUNCTIONS=========
float getFreq(float pTime) { return (1/pTime); }

float getWindMPH(float freq) { return (freq*2.5); }

float getWindMS(float freq) { return (freq*2.5*0.447); }

float getAvgWindSpeedMS(float cPulse, int per) {
  if (per) return getWindMS(getFreq((float)(cPulse/per)));
  else return 0;
  }

void anemometerISR() {
  unsigned long cTime = millis();               //get current time
  if(start != true and cTime > sTime) {         //this if statement won't run for first detection of falling edge. Will start after first one since start will be set to false.
    //the "cTime > sTime" statement is important to prevent double counts due to hardware debounce of relay in anemometer
    pulseTime = (float)(cTime - sTime)/1000;    
    culPulseTime += pulseTime;
    avgWindCount++;
  }
  sTime = cTime;
  start = false;
  if (LED== LOW) digitalWrite(LED, HIGH);
  else digitalWrite(LED, LOW);
}

void updateLCD(float windSpeed) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("avg wind speed");
  lcd.setCursor(0, 1);
  lcd.print(windSpeed);
  lcd.setCursor(5, 1);
  lcd.print("m/s");
}

void saveData(String dataString){
  //if(SD.exists("data.csv")){ // check the card is still there
  File sensorData = SD.open("data.csv", FILE_WRITE);
  if (sensorData){
      sensorData.println(dataString);
      Serial.println("Wrote to file.");
      sensorData.close(); // close the file
    }
  //}
  else{
    Serial.println("Error writing to file !");
  }
}
