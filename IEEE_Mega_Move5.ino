//no minerals in the way, no brush
// consider the reorientation of the robot at nebulite to be a bit shorter when it moves forward
//3 loading locations
//dumping at nebulite with screw and stepper simultaneously
//sweep little corner so that we can back up properly and push the geodinium container

#include <Wire.h>

#include <NewPing.h>

#define TRIGGER_PIN 39
#define ECHO_PIN1 37
#define ECHO_PIN2 35
#define ECHO_PIN3 33
#define MAX_DISTANCE 200


int num1 = 0;
#define interruptPin 2
#define loaderRaise 43
#define loaderLower 41
#define brush 49
#define screw 45
#define stepMotor 47
#define SW1 25



#define loaderDelay 7500

NewPing frontSonar(TRIGGER_PIN, ECHO_PIN1, MAX_DISTANCE);
NewPing rightSonar(TRIGGER_PIN, ECHO_PIN2, MAX_DISTANCE);
NewPing leftSonar(TRIGGER_PIN, ECHO_PIN3, MAX_DISTANCE);

int instructionArray[] = {0, 1, 2, 3, 4, 9, 10};
int testDataArray[] = {0, 10000, 10000, 10000, 10000, 800, 800};
int targetStop = 0; //holds the obstacleFront() value result
int targetStop1 = 0;
void setup() {
  // put your setup code here, to run once:
  pinMode(A1, OUTPUT);
  digitalWrite(A1, HIGH);
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

  //int obstacleFront(): returns distance in cm to obstacle in front of the front ultrasonic sensor
  //int obstacleLeft(): returns distance in cm to obstacle in front of the left ultrasonic sensor
  //int obstacleRight(): returns distance in cm to obstacle in front of the right ultrasonic sensor

  //void waitForSwitch(): waits until SW1 on top PCB is flipped. Use to prevent robot from running while programming.



//Test Rotating Functions: 75 degrees looks really good for a 90 degree rotate
//  waitForSwitch();
//  rotateClockwise(750);
//  stopMovement();
//  delay(2000);
//  rotateCounterClockwise(750);
//  stopMovement();
//  delay(2000);
  

//Testing Ultrasound so that the robot stops when Front ultrasound is 10cm away from the wall
//  waitForSwitch();
//  targetStop = obstacleFront();
//  Serial.print("Distance from wall in cm: ");
//  Serial.println(targetStop);
//  moveForward(5000);
//  if (targetStop == 10){
//    stopMovement();
//    delay(10000);
//  }

//Testing Side Ultrasound: Loader rubs against the track and skews movement but ultrasounds work alright
//  targetStop = obstacleLeft();
//  targetStop1 = obstacleRight();
//
//  waitForSwitch();
//  moveRight(1000);
//  delay(3000);
//  moveLeft(1000);
//  delay(3000);
  
//  if (targetStop1 <= 10){
//    stopMovement();
//    delay(3000);
//    moveLeft(5000);
//  }



// ACTUAL SWEEP
// Directions are in mm and angles are in tenths. Ex// moveForward(500) = 5cm,  rotateClockwise(900) = 90 degrees. NOTE that 750 is a nice sharp rotate 90 degrees
   waitForSwitch();
//   delay(500);
//   moveForward(350);
//   delay(3000);
//   rotateClockwise(800);
//   delay(3000);
//   moveBackward(228);
//   delay(3000);
//   moveRight(50);
//   delay(1000);
//   startScrew();
//   delay(10000);
//   stopScrew();
//   delay(1000);
//   delay(20000);
//--------------------------------
   delay(500);
   startBrush();
   delay(1000);
   moveForward(200);
   delay(2000);
   rotateCounterClockwise(750);
   delay(3000);
   rotateCounterClockwise(800);
   delay(3000);
   moveForward(228);
   delay(2000);
   rotateClockwise(750);
   delay(3000);
   moveBackward(101);
   delay(1000);
   moveForward(120);
   delay(2000);
   rotateClockwise(800);// angle to hit the cave wall
   delay(3000);
   moveForward(380);
   delay(3000);//
   rotateCounterClockwise(750);//
   delay(3000);
//After Collection, dump into sorting mechanism mid sweep
   raiseLoader();
   delay(1000);
   lowerLoader();
   delay(1000);
   moveForward(250); //sweep that corner so we can back up into it properly.
   delay(3000);
   moveBackward(100);
   delay(1000);
   rotateCounterClockwise(700);//
   delay(3000);
   moveForward(370);// move forward from cave wall to the Geodinium crate
   delay(3000);
   rotateCounterClockwise(700);//angle to back into wall near Geodinium container
   delay(3000);
   moveBackward(508);
   delay(3000);
   moveRight(120);
   delay(1000);
   moveBackward(101);
   delay(1000);
//Move to and Dump minerals outside of robot into the container next to origin
   moveForward(345);//make sure it runs into the wall 270 degress initially
   delay(3000);
//   moveForward(381);
//   targetStop = obstacleFront();
//   if(targetStop <= 10){
//    stopMovement();
//   }
//   delay(3000);
   rotateClockwise(850);//angle to line up against cave wall next to container
   delay(3000);
   moveBackward(228);
   delay(3000);
   moveLeft(100);
   delay(1000);
   moveBackward(101);
   delay(1000);
   moveRight(110); //make sure its close to the container
   delay(1000);
   moveBackward(101);
   delay(1000);
   raiseLoader();
   delay(2000);
   lowerLoader();
   delay(2000);
   startStep();
//   delay(500);
//   moveLeft(25); //jostle the loader and sorting mechanic
//   delay(250);
//   moveRight(25);
//   delay(250);  
//   moveLeft(25); //jostle the loader and sorting mechanic
//   delay(250);
//   moveRight(25);
   delay(8000);
   stopStep();
   delay(500);
   //Enter Cave backwards and then come back out
   moveLeft(130);// strafe so that we are in center of cave to back into 130 BEFORE
   delay(3000);
   moveBackward(135);
   delay(2000);
   //Go to the container, back up against wall and dump into Nebulite container
   moveForward(650);// forward to go outside wave and stop by the right of the Neb container
   delay(4500);
//   moveLeft(241);
//   delay(3000);
//   moveBackward(101);
//   delay(1000);
   rotateCounterClockwise(750);//angle to back into wall near Geodinium container
   delay(3000);
   moveBackward(508);
   delay(3000);
   moveRight(120);//strafe torward the geodinium box REMEMBER THAT IT WAS PUSHED WHEN WE FIRST GOT THERE
   delay(2000);
   moveBackward(101);
   delay(1000);
   raiseLoader();
   delay(2000);
   lowerLoader();
   delay(2000);
   startScrew();
   delay(500);
   startStep();
   delay(8000);
   stopScrew();
   delay(500);
   stopStep();
   delay(1000);
   //Reorient behind Nebulite container and push it to rendevouz pad
   moveForward(160); // moving forward to rotate and lineup  with cave wall 165 BEFORE
   delay(3000);
   rotateClockwise(750);
   delay(3000);
   moveBackward(200);
   delay(3000);
   moveLeft(228);
   delay(3000);
   moveBackward(101);
   delay(1000);
   stopBrush();
   delay(500);
   raiseLoader();
   delay(3000);
   moveForward(600);//go straight to the rendevous pad
   delay(3000);
   stopMovement();
   delay(1000);
   moveBackward(150);// move back to lower loader
   delay(2000);
   stopMovement();
   delay(1000);
   lowerLoader();
   delay(3000);
   //End of Sweep
   stopMovement();
   delay(500);
   delay(20000);


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

int obstacleFront()
{
  return (frontSonar.ping_cm());
}

int obstacleLeft()
{
  return (leftSonar.ping_cm());
}

int obstacleRight()
{
  return (rightSonar.ping_cm());
}

void waitForSwitch()
{
  while (digitalRead(SW1) == HIGH)
  {
    delay(10);
  }
}
