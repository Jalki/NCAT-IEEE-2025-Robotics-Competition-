#include <Wire.h>

int num1 = 0;
#define interruptPin 2
#define loaderRaise 43
#define loaderLower 41
#define brush 49
#define screw 45
#define stepMotor 47

#define loaderDelay 7500

int instructionArray[] = {0, 1, 2, 3, 4, 9, 10};
int testDataArray[] = {0, 10000, 10000, 10000, 10000, 800, 800};
void setup() {
  // put your setup code here, to run once:
  
  pinMode(interruptPin, OUTPUT);
  pinMode(loaderLower, OUTPUT);
  pinMode(loaderRaise, OUTPUT);
  pinMode(brush, OUTPUT);
  pinMode(screw, OUTPUT);
  pinMode(stepMotor, OUTPUT);
  Serial.begin(1000000);
  Wire.begin(20);
  delay(500);

}

void loop() {
  // put your main code here, to run repeatedly:


  // LIST OF FUNCTIONS:
  //void stopMovement(): stops robot
  //void moveForward(int distance):  distance is in mm, maximum value is 0x7FFF mm, maximum positive signed 16 bit integer
  //void moveBackward(int distance): distance is in mm
  //void moveLeft(int distance): distance is in mm
  //void moveRight(int distance): distance is in mm
  //void rotateClockwise(int angle): angle is in tenths of a degree, e.g. rotateClockwise(900) will rotate approx. 90 degrees. Adjust as needed.
  //void rotateCounterClockwise(int angle): angle is in tenths of a degree, e.g. rotateClockwise(900) will rotate approx. 90 degrees. Adjust as needed.
  // ** MOVEMENT FUNCTIONS ARE INSTANTANEOUS, ADD A DELAY BEFORE THE NEXT INSTRUCTION **

  //void raiseLoader(): raises loader, takes a short time to execute function
  //void lowerLoader(): lowers loader, takes a short time to execute function
  //void startBrush(), void stopBrush(): starts/stops brush on loader
  //void startStep(), void stopStep(): starts/stops step motor in sorter
  //void startScrew(), void stopScrew(): starts/stops screw motor in sorter
          

  lowerLoader();
  startBrush();
  moveForward(500);
  delay(2000);
  stopBrush();
  stopMovement();
  raiseLoader();
  delay(500);
  lowerLoader();
  startStep();
  delay(5000);
  stopStep();
  startScrew();
  delay(10000);
  stopScrew();


  // Do not change the program on the BLE. Do not modify the code for the functions. Create a routine by calling the functions.

}


void  moveForward(int distance)
{
  Wire.beginTransmission(21);
  Wire.write(1);
  Wire.write((distance & 0xFF00) / 256 );
  Wire.write(distance & 0x00FF); 
  Wire.endTransmission();
}
void moveBackward(int distance)
{
  Wire.beginTransmission(21);
  Wire.write(2);
  Wire.write((distance & 0xFF00) / 256 );
  Wire.write(distance & 0x00FF); 
  Wire.endTransmission();
}
void stopMovement()
{
  Wire.beginTransmission(21);
  Wire.write(0);
  Wire.write(1);
  Wire.write(1);
  Wire.endTransmission();
  
}
void moveRight(int distance)
{
  Wire.beginTransmission(21);
  Wire.write(4);
  Wire.write((distance & 0xFF00) / 256 );
  Wire.write(distance & 0x00FF); 
  Wire.endTransmission();
}

void moveLeft(int distance)
{
  Wire.beginTransmission(21);
  Wire.write(3);
  Wire.write((distance & 0xFF00) / 256 );
  Wire.write(distance & 0x00FF); 
  Wire.endTransmission();
}

void raiseLoader()
{
  digitalWrite(loaderRaise, HIGH);
  delay(loaderDelay);
  digitalWrite(loaderRaise, LOW);
}

void lowerLoader()
{
  digitalWrite(loaderLower, HIGH);
  delay(loaderDelay);
  digitalWrite(loaderLower, LOW);
}

void startBrush()
{
  digitalWrite(brush, HIGH);
}

void stopBrush()
{
  digitalWrite(brush, LOW);
}

void startScrew()
{
  digitalWrite(screw, HIGH);
}

void stopScrew()
{
  digitalWrite(screw, LOW);
}

void startStep()
{
  digitalWrite(stepMotor, HIGH);
}

void stopStep()
{
  digitalWrite(stepMotor, LOW);
}

void rotateClockwise(int angle)
{
  Wire.beginTransmission(21);
  Wire.write(9);
  Wire.write((angle & 0xFF00) / 256 );
  Wire.write(angle & 0x00FF); 
  Wire.endTransmission();
}

void rotateCounterClockwise(int angle)
{
  Wire.beginTransmission(21);
  Wire.write(10);
  Wire.write((angle & 0xFF00) / 256 );
  Wire.write(angle & 0x00FF); 
  Wire.endTransmission();
}
