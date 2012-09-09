



// Serial R/C Car control sketch
// Demo for GT MAKE club at CCB 337
// 2012 Sterling Peet <sterling.peet@gatech.edu> 
// William Kurkian <wkurkian3@gatech.edu>

#include <AFMotor.h>
#include <Servo.h>
#include "structures.h"
#include <EEPROM.h>

// Which Adafruit Motor Driver are we using?
const int driveMotorLocation = 3;
// Where is the steering servo plugged in?
const int steeringServo = 9;
const int motorTimeout = 500; // milliseconds

int defaultAngle = 87;
int turnAngle = defaultAngle;
int motorSpeed = 0;
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
int servoFactor;
int servoMin;

int centerAddress = 0;

boolean debugMode = true;

int freeDegrees = 50;

int lastTime;

AF_DCMotor driveMotor(driveMotorLocation);
Servo steering;

boolean motorOn;

int motorLimit = 140;
int turnLimit = 50;

void setup() {
  Serial.begin(115200);  // set up Serial library
  Serial.println("Demo R/C Car Controller");
  // reserve 20 bytes for the inputString:
  inputString.reserve(20);
  // turn on servo
  steering.attach(steeringServo);
  turnAngle = EEPROM.read(centerAddress);
  steering.write(turnAngle);
  
  // turn on motor
  driveMotor.run(RELEASE);
  lastTime = millis();
  
  motorOn = false;
  
  //to transform a number from 0-180 to 40 - 140
  //x*servoFactor + servoMin
  servoFactor = 5.0/9;
  servoMin = 40;
  
}

Command * getCommands(String inputStringIn) {
  int i = 0;
  Command commands[6];
  int index = 0;
  while (i < inputStringIn.length()) {
      
    if (inputStringIn.charAt(i) == '(') {
      char commandChar = inputStringIn.charAt(i+1);
      int value = (inputStringIn.charAt(i+2)- '0') *100 + (inputStringIn.charAt(i+3)- '0')*10 + (inputStringIn.charAt(i+4)-'0');
      if (index < 6) {
       Command com;
       com.command = commandChar;
       com.value = value;
       commands[index] = com;
       index++;
      }
    }
    i++;
    
  }
  
  return commands;
}

void loop() {
  int timeNow = millis();
  int steeringTime, motorTime;
  if (motorOn) {
     delay(100);
  }
  // print the string when a newline arrives:
  
  // Check to see if we need to change the drive or steering states
  // Check and see if we need to turn off motor or servo?
  if ((timeNow - lastTime) > 500 && motorOn) {
    if (! debugMode) {
      driveMotor.run(RELEASE);
      motorOn = false;
    }
    
  }
  if (stringComplete && (timeNow -lastTime ) > 50) {
    
    Command * commands = getCommands(inputString);
    lastTime = millis();
    int end = 1;
    int i;
    for (i = 0; i < end; i++) {
      //if (commands[i] == NULL)
        //break;
      char command = commands[i].command;
      int value = commands[i].value;
      if (command == 'M') {
         Serial.println("forward");
         if (commands[i].value <= motorLimit) {
           driveMotor.setSpeed(commands[i].value );
           driveMotor.run(FORWARD);
         }
         motorOn = true;
         
      } else if (commands[i].command == 'S') {
         Serial.println("turn");
         turnAngle = commands[i].value;
         if (turnAngle > 180)
           turnAngle = 180;
         turnAngle = (int)(turnAngle * servoFactor + servoMin);
         Serial.println(turnAngle);
         steering.write(turnAngle);
         
      }  else if (commands[i].command == 'Q') {
         defaultAngle = commands[i].value;
         if (defaultAngle > 180)
             defaultAngle = 180;
         defaultAngle = defaultAngle*servoFactor + servoMin;
         steering.write(defaultAngle);
         Serial.println(defaultAngle);
         EEPROM.write(centerAddress,defaultAngle);
      } else if (commands[i].command == 'X') {
        driveMotor.run(RELEASE); 
        motorOn = false;
      }
      
    }
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
}


/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
      
    }
    Serial.println("done");
  }
}

