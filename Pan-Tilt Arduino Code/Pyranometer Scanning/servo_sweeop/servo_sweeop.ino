/* Sweep
  by BARRAGAN <http://barraganstudio.com>
  This example code is in the public domain.

  modified 8 Nov 2013
  by Scott Fitzgerald
  http://www.arduino.cc/en/Tutorial/Sweep
*/

#include <Servo.h>

Servo panservo;
Servo tiltservo; // create servo object to control a servo
// twelve servo objects can be created on most boards

int16_t panpos = -90;
int16_t tiltpos = -90;
byte steps = 0;

// variable to store the servo position

void setup() {
  panservo.attach(9);// attaches the servo on pin 9 to the servo object
  tiltservo.attach(10);
  panservo.write(pan_map_angle(-90)); //halfway point
  tiltservo.write(tilt_map_angle(-90));// halfway point
}
void loop() {
  for (panpos = -90; panpos <= 90; panpos += 10) {
    for (tiltpos = -90; tiltpos <=  90; tiltpos += 5) { // goes from 0 degrees to 180 degrees (-90 to 90)
      // in steps of 5 degree
      tiltservo.write(tilt_map_angle(tiltpos));
      delay(150);
    }
    //take analog measurement
    for (tiltpos = 90; tiltpos >= -90; tiltpos -=  10) { //then go back
      tiltservo.write(tilt_map_angle(tiltpos));
      delay(15);
    }
    panservo.write(pan_map_angle(panpos));
    // tell servo to go to position in variable 'pan-pos'

    // waits 15ms for the servo to reach the position
  }
}

int ReadSens_and_Condition() {
  int i;
  byte numReadings = 10;
  byte readingDelay = 10;
  int accum = 0;

  for (i = 0; i < numReadings; i++) {
    accum += analogRead(0); // sensor on analog pin 0
    delay(readingDelay);
  }

  byte avg = accum / numReadings;
  return avg;// return avg to specific index
}

int16_t tilt_map_angle(int16_t angle) {
  angle = map(angle, -135, 135, 0, 180);
  return angle;
}

int16_t pan_map_angle(int16_t angle) {
  angle = map(angle, -135, 135 , 0, 180);
  return angle;
}

//byte indextoAngle(byte pan-index, byte tilt-index){
//}


//}
