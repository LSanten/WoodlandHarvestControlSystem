

/*******************************************************************************************************************************************************************************************************************************************
 * Title: Anemometer and data logging for classic Vortex anemometer
 * Description: measures wind speed, saves data to SD card
 * Info: watch this video on measuring wind speed if questions: https://www.youtube.com/watch?v=emE6yWWQUHg
 * Created by Leon Santen (leon.santen@icloud.com ), Fall 2020
 *******************************************************************************************************************************************************************************************************************************************
 */
#include <Wire.h>
#include <config.h>             //for real time clock
#include <ds3231.h>             //real time clock library from https://github.com/rodan/ds3231 //tutorial on https://create.arduino.cc/projecthub/MisterBotBreak/how-to-use-a-real-time-clock-module-ds3231-bc90fe
                                //look in comments of arduino page if error is DS3231_INTCN' was not declared in this scope
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>

// Wiring: SDA pin is connected to A4 and SCL pin to A5.
// Connect to LCD via I2C, default address 0x27 (A0-A2 not jumpered)
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2); // Change to (0x27, 20, 4)) for 20x4 LCD.

struct ts t;                              //objects for real time clock
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

//======================SETTINGS========================
uint32_t dataLoopLength = 10000;    // length of loop to report average speed
String filenameSD = "wind";       // will automatically be set to date - filename without .csv at the end filenameSD to write to SD card, can't be longer than 8
//======================SETTINGS========================


//=============SETUP================================================================
void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  pinMode(interruptPin, INPUT);                  //set interrupt pin to input

  //LCD startup 
  lcd.init();           //initiate lcd display
  lcd.backlight();
  lcd.setCursor(0, 0); // Set the cursor on the first column and first row.
  lcd.print("   Anemometer   "); // Print the string "Hello World!"

  //REAL TIME CLOCK SETUP
  //check out for setting time https://create.arduino.cc/projecthub/MisterBotBreak/how-to-use-a-real-time-clock-module-ds3231-bc90fe
  Wire.begin();
  //DS3231_init(DS3231_INTCN);
  DS3231_init(0);
  DS3231_get(&t);             //get current time
  //display real from rtc on lcd display  
  
  //SETUP SD CARD
  Serial.begin(9600);
  Serial.println("Initializing SD card...");
  pinMode(CSpin, OUTPUT);                        //CS pin for SD card writing
  if (!SD.begin(CSpin)) {
    Serial.println("Card failed, or not present.");
    lcd.setCursor(0, 1); //Set the cursor on the third column and the second row (counting starts at 0!).
    lcd.print("SD card failed");
    return;
  }
  else{
    Serial.println("card initialized.");
    lcd.setCursor(0, 1); //Set the cursor on the third column and the second row (counting starts at 0!).
    lcd.print("SD card found");
  }  
  
  filenameSD = changeFileNameIfExists(filenameSD);
  
  
  String dataString = String("Speed [m/s]"); //write CSV (comma seperated vector)
  String timeString = String("Time [s]") + ",";
  String dateString = String("Date") + ",";
  saveData(dateString, timeString, dataString, filenameSD);
   
  
  //SETUP END
  Serial.end();
  lcd.clear();
  
  //attachInterrupt(digitalPinToInterrupt(interruptPin), anemometerISR, RISING); //setup interrupt on anemometer input pin, will run anemometerISR function whenever falling edge is detected
  dataTimer = millis();                                 //start data timer

  
}
//=====END=====SETUP================================================================

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
    DS3231_get(&t);             //get current time
    //display real from rtc on lcd display
    String timeString = String(t.hour) + String(":") + String(t.min) + String(":")+ String(t.sec) + String(",");
    String dateString = String(t.mon) + String("-")+ String(t.mday) + String(",");                                //string can't be longer - otherwise memory is too full
  
    
    dataString = String(aWSpeedMS); //write CSV (comma seperated vector)
    saveData(dateString, timeString, dataString, filenameSD);
    

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

void addTableHeader(String tableHeader, String filenameSD){
  File sensorData = SD.open(filenameSD, FILE_WRITE);
  sensorData.println(tableHeader);
  Serial.println("wrote header to table.");
  sensorData.close(); // close the file
}

void saveData(String dateString, String timeString, String dataString, String filenameSD){
  //if(SD.exists(filenameSD)){ // check the card is still there
  File sensorData = SD.open(filenameSD, FILE_WRITE);
  if (sensorData){
      sensorData.print(dateString);                 //strings need to be parsed individually - otherwise they are too long for limited memory
      sensorData.print(timeString);
      sensorData.println(dataString);
      Serial.println("Wrote to file.");
      sensorData.close(); // close the file
    }
  //}
  else{
    Serial.println("Error writing to file !");
  }
}

//void saveDataWithoutFileOverwrite(String dataString){
  
//}

String changeFileNameIfExists(String filenameSD){
  //add number to filename if filename exists and add .csv
  String fullFilename = String(filenameSD) + String(".csv");
  int numberAddOn = 0;
  
  if(SD.exists(fullFilename)){
    Serial.print(fullFilename);
    Serial.println(" exists already.");

    while(SD.exists(fullFilename)){
      fullFilename = String(filenameSD) + String("_") + String(numberAddOn) + String(".csv");
      numberAddOn = numberAddOn + 1;
      Serial.println("thinking about new filenames...");
    }

    Serial.print(fullFilename);
    Serial.println(" will be used.");
    return(fullFilename);
  }
  else{
    String message = String("Filename ") + String(fullFilename) + String(" doesn't exists");
    Serial.println(message); 
    Serial.println("File will be created.");
    return(fullFilename);
  }
}
