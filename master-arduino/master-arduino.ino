/*******************************************************************************************************************************************************************************************************************************************
 * Title: Communication System for Main House Electricity System at Woodland Harvest Mountain Farm
 * Description: 
 * What does code do?: 
 * Harware wanings: 
 * Created by Riley Zito, Odalys Benitez, Leon Santen (leon.santen@icloud.com ), Fall 2020
 *******************************************************************************************************************************************************************************************************************************************
 */

// TODO: Add directions for future work on coce here in this section

//==========================================================================================================================================================================================================================================
// Load supporting Arduino Libraries
//==========================================================================================================================================================================================================================================
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//==========================================================================================================================================================================================================================================
// Create and initialize global variables, objects, and constants (containers for all data)
//==========================================================================================================================================================================================================================================
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 20, 4); // Change to (0x27, 20, 4)) for 20x4 LCD.

const int aliveLED = 13;                //create a name for "robot alive" blinky light pin
const int eStopPin = 12;                //create a name for pin connected to ESTOP switch
const int PWR_STRIP = 4;                //pin for power strip switch
const int buzzerPin = 5;                //pin for piezo buzzer
boolean aliveLEDState = true;           //create a name for alive blinky light state to be used with timer
boolean ESTOP = true;                   //create a name for emergency stop of all motors
boolean realTimeRunStop = true;         //create a name for real time control loop flag
String loopError = "no error";          //create a String for the real time control loop error system
unsigned long oldLoopTime = 0;          //create a name for past loop time in milliseconds
unsigned long newLoopTime = 0;          //create a name for new loop time in milliseconds
unsigned long cycleTime = 0;            //create a name for elapsed loop cycle time
const long controlLoopInterval = 120;  //create a name for control loop cycle time in milliseconds
int buzzerState = 0;                   //state through which we iterate to play different sounds after each other
bool lastLoopPStrip = false;            //boolean that tells you if fride power strip was turned on during last loop

int voltageDivider = A0;                //create a name for pin for input voltage from voltage divider
int voltDivValue = 0;
float batteryVoltage = 0;
int averageVoltCounter = 0;             //increments every revolution to count up
float averageVoltage = 0;               //averaged Voltage
float voltageSumUp = 0;                 //place to sum up voltages which will be devided by count to get average

uint32_t fridgeWaitTimerStart = 0;          // millis start time of timer for fridge to wait before checking voltaget (which might turn fridge off again)
uint32_t fridgeTimer;

int displayState = 1;                   //states for display visuals


//===================SETTINGS======================
bool defaultSwitch = true;                        //true turns on running on a default setting, false waits for your input.
String command = "fridge";                        //will default to this command if defaultSwitch == true
float fridgeUpperVoltageThreshold = 28.8;        //fridge will turn on when this threshold is reached
float fridgeLowerVoltageThreshold = 26.35;         //fridge will turn off when this lower threshold is reached (fridge won't turn off if this voltage is reached during the first two minutes after is was switched on)
//===================SETTINGS======================

//==========================================================================================================================================================================================================================================
// Startup code to configure robot and pretest all robot functionality (to run once)
// and code to setup robot mission for launch
//==========================================================================================================================================================================================================================================
void setup() {
  // Step 1) Put your robot setup code here, to run once:
  pinMode(aliveLED, OUTPUT);          //initialize aliveLED pin as an output
  pinMode(eStopPin, INPUT_PULLUP);    //use internal pull-up on ESTOP switch input pin 
  pinMode(buzzerPin, OUTPUT);
  Serial.begin(9600);                 //start serial communication
  Serial.println(" Controller Starting Up! Make sure you followed the Olin-at-Woodland-Harvest-Electricity-Guidelines ");

  // Step 2) Put your robot mission setup code here, to run once:
  pinMode(PWR_STRIP, OUTPUT);
  digitalWrite(PWR_STRIP, LOW);

  // beep twice during boot up
  tone(buzzerPin, 3500);
  delay(250);
  tone(buzzerPin, 4200);
  delay(250);
  noTone(buzzerPin);

  //LCD DISPLAY INIT
  lcd.init();           //initiate lcd display
  lcd.backlight();
  lcd.setCursor(0, 0);  // Set the cursor on the first column and first row.

}
//==========================================================================================================================================================================================================================================
// Flight code to run continuously until robot is powered down
//==========================================================================================================================================================================================================================================
void loop() {
  // Step 3) Put Operator-Input-to-Robot and Robot_Reports-Back-State code in non-real-time "outer" loop:
  //         Put real-time dependant sense-think-act control in the inner loop

  // GET Operator Control Unit (OCU) Input: ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu-------
  command = getOperatorInput();                         //get operator input from serial monitor
  if (command == "stop") realTimeRunStop = false;       //skip real time inner loop
  else realTimeRunStop = true;                          //set loop flag to run = true

  // Step 4) Put your main flight code into "inner" soft-real-time while loop structure below, to run repeatedly,
  //         at a known fixed "real-time" periodic interval. This "soft real-time" loop timing structure, runs
  //         fast flight control code once every controlLoopInterval.

  // real-time-loop*******real-time-loop*******real-time-loop*******real-time-loop*******real-time-loop*******
  // real-time-loop*******real-time-loop*******real-time-loop*******real-time-loop*******real-time-loop*******
  while(realTimeRunStop == true) {

    // Check if operator inputs a command during real-time loop execution
    if (Serial.available() > 0) {                             // check to see if operator typed at OCU
      realTimeRunStop = false;                                // if OCU input typed, stop control loop
      command = Serial.readString();                          // read command string to clear buffer
      break;                                                  // break out of real-time loop
      }
    else {realTimeRunStop = true;}                            // if no operator input, run real-time loop

    // Real-Time clock control. Check to see if one clock cycle has elapsed before running this control code
    newLoopTime = millis();                                   // get current Arduino time (50 days till wrap)
    if (newLoopTime - oldLoopTime >= controlLoopInterval) {   // if true run flight code
      oldLoopTime = newLoopTime;                              // reset time stamp
      blinkAliveLED();                                        // toggle blinky alive light



    // SENSE sense---sense---sense---sense---sense---sense---sense---sense---sense---sense---sense---sense---sense-------------
    // TODO add sensor code here

    // THINK think---think---think---think---think---think---think---think---think---think---think---think---think-------------
    // Pick robot behavior based on operator input command typed at console
      uint32_t currentTime = millis();                            //time of loop
      //Serial.print("CurrentT: ");
      //Serial.println(currentTime);
            
      if ( command == "stop") {
        Serial.println("Stop Robot");
        realTimeRunStop = false;                                  // exit real-time control loop
        break;
      }
      else if (command == "measure" or command == "m") {
        // READ BATTERY VOLTAGE
        voltDivValue = analogRead(voltageDivider);                // read value from voltage divider 
        batteryVoltage = 6.097*(float(voltDivValue)/205);         // multiply value by 6 to get real voltage value
        //Serial.println(batteryVoltage);

        // AVERAGE VOLTAGE
        int avgCount = 30;
        if (averageVoltCounter <= avgCount){
          ++averageVoltCounter;                                   // increment counter
          voltageSumUp = voltageSumUp + batteryVoltage;       // add up voltages to divide by count later
          if (averageVoltCounter == avgCount){
            
            averageVoltage = voltageSumUp/avgCount;
            Serial.print("-->");
            Serial.println(averageVoltage);
            averageVoltCounter = 0;
            voltageSumUp = 0;            
          }
        }
        realTimeRunStop = true;                                   // run loop continually        
      }
      
      else if (command == "fridge" or command == "f"){                                // make robot alive with small motions
        // READ BATTERY VOLTAGE
        voltDivValue = analogRead(voltageDivider);                // read value from voltage divider 
        batteryVoltage = 5.78*(float(voltDivValue)/205);         // multiply value by 6 to get real voltage value
        //Serial.println(batteryVoltage);

        // AVERAGE VOLTAGE
        int avgCount = 100;
        if (averageVoltCounter <= avgCount){
          ++averageVoltCounter;                                   // increment counter
          voltageSumUp = voltageSumUp + batteryVoltage;       // add up voltages to divide by count later
          if (averageVoltCounter == avgCount){
            
            averageVoltage = voltageSumUp/avgCount;
            Serial.print("==> avg. voltage: ");
            Serial.println(averageVoltage);
            averageVoltCounter = 0;
            voltageSumUp = 0;   

            updateDisplay(displayState); //display system voltage and thesholds
          }
        }

        // SWITCH FRIDGE BASED ON VOLTAGE
        
        // PRINT FRIDGE STATUS
        if (digitalRead(PWR_STRIP) == HIGH and averageVoltCounter == 1){
          
          Serial.println("==> fridge ON");
        }
        else if (digitalRead(PWR_STRIP) == LOW and averageVoltCounter == 1){
          Serial.println("==> fridge OFF");
        }

        // CHECK VOLTAGE AND TURN FRIDGE ON IF CURRENTLY OFF AND ABOVE UPPER VOLTAGE THRESHOLD 
        if (averageVoltage > fridgeUpperVoltageThreshold and digitalRead(PWR_STRIP) == LOW){
          Serial.println("======");
          
          fridgeWaitTimerStart = millis();          
          Serial.print("Timer: ");
          Serial.println(fridgeTimer);
          Serial.println("Fridge turned ON");
          Serial.print("avgVoltage ");
          Serial.println(averageVoltage);
          Serial.print("threshold ");
          Serial.println(fridgeUpperVoltageThreshold);
          digitalWrite(PWR_STRIP, HIGH);          
        }

        // This if statement makes sure that fridgeWaitTimerStart is not larger than currentTime. If it was, it would create a negative number which is just a large number in uint32_t
        if (fridgeWaitTimerStart > currentTime){             
          fridgeTimer = 0;
        }
        else{
          fridgeTimer = currentTime - fridgeWaitTimerStart;
          //Serial.print("Timer: ");
          //Serial.println(fridgeTimer);
        }
                  
        
        if(averageVoltage < fridgeLowerVoltageThreshold and fridgeTimer > 180000){
          
          Serial.println("======");
          Serial.print("Timer: ");
          Serial.println(fridgeTimer);
          
          Serial.println("Fridge OFF");
          Serial.print("avgVoltage ");
          Serial.println(averageVoltage);
          Serial.print("threshold ");
          Serial.println("======");
          Serial.println(fridgeLowerVoltageThreshold);
          digitalWrite(PWR_STRIP, LOW);
          fridgeTimer = 0;
        }
        realTimeRunStop = true;                                   // run loop continually

        //BUZZER SECTION
        //detect when power strip goes from on to off and off to on
        if(digitalRead(PWR_STRIP) == HIGH and lastLoopPStrip == false){
          buzzerState = 1;
          lastLoopPStrip = true;
        }        
        else if(digitalRead(PWR_STRIP) == LOW and lastLoopPStrip == true){
          buzzerState = 33;
          lastLoopPStrip = false;
        }

        //play through tones based on off or on
        if (buzzerState >= 1 and buzzerState <=10){             //fridge got turned on
          tone(buzzerPin, 3000);
          buzzerState = buzzerState + 1;
          Serial.println("added one");
        }        
        else if (buzzerState >= 11 and buzzerState <=20){
          tone(buzzerPin, 3300);
          buzzerState = buzzerState + 1;
        }
        else if (buzzerState >= 21 and buzzerState <=30){
          tone(buzzerPin, 3500);
          buzzerState = buzzerState + 1;
        }
        else if (buzzerState == 31){
          noTone(buzzerPin);
          buzzerState = 32;
        }
        else if (buzzerState >= 33 and buzzerState <=42){
          tone(buzzerPin, 3500);
          buzzerState = buzzerState + 1;
        }
        else if (buzzerState >= 43 and buzzerState <=52){
          tone(buzzerPin, 3300);
          buzzerState = buzzerState + 1;
        }
        else if (buzzerState >= 53 and buzzerState <=62){
          tone(buzzerPin, 3000);
          buzzerState = buzzerState + 1;
        }
        else if (buzzerState == 63){
          noTone(buzzerPin);
          buzzerState = buzzerState + 1;
        }

        

      }
      
      

      else if (command == "idle"){                                // make robot alive with small motions
        Serial.println("Idle Robot");
        Serial.println("Type stop to stop robot");
        realTimeRunStop = true;                                   // run loop continually
      }
      else
      {
        Serial.println("**** WARNING **** Invalid Input, System stopped, Please try again!");
        realTimeRunStop = false;
      }

    // ACT act---act---act---act---act---act---act---act---act---act---act---act---act---act---act---act---act---act-----------
       ESTOP = digitalRead(eStopPin);                                 // check ESTOP switch

    // Check to see if all code ran successfully on one real-time increment
    cycleTime = millis()-newLoopTime;                                 // calculate loop execution time
    if( cycleTime > controlLoopInterval){
      Serial.println("********************************************");
      Serial.println("error - real-time has failed, stop robot!");    // loop took too long to run
      Serial.print(" 1000 ms real-time loop took = ");
      Serial.println(cycleTime);                                      // print loop time
      Serial.println("********************************************");
      break;
    }
    } // end of "if (newLoopTime - oldLoopTime >= controlLoopInterval)" real-time loop structure
  } // end of "inner"   "while(realTimeRunStop == true)" real-time control loop
  // real-time-loop*******real-time-loop*******real-time-loop*******real-time-loop*******real-time-loop*******
  // real-time-loop*******real-time-loop*******real-time-loop*******real-time-loop*******real-time-loop*******

  // SEND Robot State to Operator Control Unit (OCU) ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu-----
    //Serial.println("  ");
    Serial.println("=======================================================================================");
    Serial.println("| Robot control loop stopping to wait for new command ");         // send robot status to operator
    if (ESTOP == true) Serial.println("| Robot motors E-Stopped by external switch"); // send E-Stop message to OCU



} // End of "outer" void loop()
//==========================================================================================================================================================================================================================================
// END OF Flight Code
//==========================================================================================================================================================================================================================================

//==========================================================================================================================================================================================================================================
//==========================================================================================================================================================================================================================================
// FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    
// Functions for each section of above code
// Please note: Except for very simple cases, it would be better to place all of these functions in a 
// myRobotControlFunctions.h file and #include it at start of program to keep robot flight code brief

//-----------------------------------------------------------------------------------------------
// Functions for setup code
//-----------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------
// Functions for flight code
//-----------------------------------------------------------------------------------------------

// Realtime loop functions loop---loop---loop---loop---loop---loop---loop---loop---loop---loop---

void blinkAliveLED(){
  // This function toggles state of aliveLED blinky light LED
  // if the LED is off turn it on and vice-versa:
    if (aliveLEDState == LOW) {
      aliveLEDState = HIGH;
    } else {
      aliveLEDState = LOW;
    }
    // set the LED with the ledState of the variable:
    digitalWrite(aliveLED, aliveLEDState);
}

// OCU functions ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu-----

String getOperatorInput() {
  // This function prints operator command options on the serial console and prompts
  // the operator to input desired robot command
  // Serial.println("   ");
  Serial.println("=======================================================================================");
  Serial.println("| Robot Behavior-Commands: move(moves robot), stop(e-stops motors), idle(robot idles) |");
  Serial.println("|                                                                                     |");
  Serial.println("|     Please type desired robot behavior in command line at top of this window        |");
  Serial.println("|     and then press SEND button.                                                     |");
  Serial.println("=======================================================================================");
  
  //                       // read command string
  if (defaultSwitch == true) {
    command = command;
  }
  else {
    while (Serial.available()==0) {};                     // do nothinguntil operator input typed
    command = Serial.readString(); 
  }
  
  Serial.print("| New Robot behavior command is: ");    // give command feedback to operator
  Serial.println(command);
  Serial.println("| Type 'stop' to stop control loop and wait for new command                           |");
  Serial.println("======================================================================================|");
  return command;
}

// SENSE functions sense---sense---sense---sense---sense---sense---sense---sense---sense---------
// place sense functions here

// THINK functions think---think---think---think---think---think---think---think---think---------
// place think functions here

// ACT functinos act---act---act---act---act---act---act---act---act---act---act---act---act-----
// place act functions here

void updateDisplay(int displayState){
  lcd.clear();

  if (displayState == 1){     //standard state
    lcd.setCursor(0, 0); // Set the cursor on the first column and first row.
    lcd.print("Fridge Settings ");
    lcd.setCursor(0, 1); // Set the cursor on the first column and first row.
    lcd.print("sys voltage: "); // Print the string "Hello World!"
    lcd.setCursor(15, 1);
    lcd.print(averageVoltage);
    lcd.setCursor(0, 2);
    lcd.print("on @: "); 
    lcd.setCursor(15, 2);
    lcd.print(fridgeUpperVoltageThreshold);
    lcd.setCursor(0, 3);
    lcd.print("off @: ");
    lcd.setCursor(15, 3);
    lcd.print(fridgeLowerVoltageThreshold);
  }
}

// END of Functions
//===============================================================================================
// END of Robot Control Code
