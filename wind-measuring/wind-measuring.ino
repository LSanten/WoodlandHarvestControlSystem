// watch this video on measuring wind speed if questions: https://www.youtube.com/watch?v=emE6yWWQUHg

int windMeter = A0;
const byte interruptPin = 2;
volatile unsigned long sTime = 0;   //stores start time for wind speed calc
unsigned long dataTimer = 0;        //used to track how often to communicate data
volatile float pulseTime = 0;       //stores time between one anemometer realy clsoing and the next
volatile float culPulseTime = 0;    //stores cumulative pusletimes for averaging
volatile bool start = true;         //tracks when a new anemometer measurement starts
volatile unsigned int avgWindCount = 0;     //stores anemometer realy countrs for doing average wind speed
float aSetting = 60.0;              //wind speed setting to signal alarm

void setup() {
  // put your setup code here, to run once:
  
  
  pinMode(interruptPin, INPUT_PULLUP); // set interrupt pin to input
  attachInterrupt(interruptPin, anemometerISR, RISING); //setup interrupt on anemometer input pin, will run anemometerISR function whenever interrupt is being triggered
  dataTimer = millis();  
}

void loop() {

  unsigned long rTime = millis();
  if((rTime - sTime) > 2500) pulseTime = 0; // if the wind speed has dropped below a certain interval time, set to 0

  if((rTime - dataTimer) > 5000){           // this is how often it will tell you the average wind speed
    detachInterrupt(interruptPin);          // shut off wind speed measurement when in this loop
    float aWSpeed = getAvgWindSpeed(culPulseTime, avgWindCount); //calcualte average wind speed
    culPulseTime = 0;     //reset cumulative pulse counter
    avgWindCount = 0;     //reset average wind count

    float aFreq = 0;      //set to zero initially
    if(pulseTime > 0.0) aFreq = getAnemometerFreq(pulseTime); //calc freq in Hz of anemomter
    float wSpeedMPH = getWindMPH(aFreq);                      //calc wind speed in MPH

    Serial.begin(9600);
    Serial.println();
    Serial.print("Hz: ");
    Serial.println(aFreq);
    Serial.print("MPH: ");
    Serial.println(wSpeedMPH);
    Serial.print("avgSpeed: ");
    Serial.println(aWSpeed);
    Serial.end();

    start = true;
    attachInterrupt(digitalPinToInterrupt(interruptPin), anemometerISR, RISING); //turn interrupt back on
    dataTimer = millis();     //reset loop timer
    
  }
}

float getAnemometerFreq(float pTime) { return (1/pTime); }

float getWindMPH(float freq) { return (freq*2.5); }

float getAvgWindSpeed(float cPulse, int per) {
  if (per) return getWindMPH(getAnemometerFreq((float)(cPulse/per)));
  else return 0;
  }

void anemometerISR() {
  unsigned long cTime = millis(); //get current time
  if(!start) {
    pulseTime = (float)(cTime - sTime)/1000;
    culPulseTime += pulseTime;
    avgWindCount++;
  }
  sTime = cTime;
  start = false;
}
