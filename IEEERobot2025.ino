int ard_sensor_check; //This is to make sure everything goes well with sensors

#include "Arduino_BMI270_BMM150.h"

#include <Wire.h>
#include <stdlib.h> 

int D1ENA1pin = 12; // Arduino pin that the motor driver IN1 pin is connected to
int D1IN1pin = 11; // Arduino pin that the motor driver IN2 pin is connected to
int D1IN2pin = 10; // Arduino PWM pin that the motor driver ENA pin is connected to 

int D1ENA2pin = 9; // Change to a different PWM pin for D1
int D1IN3pin = 8; // Arduino pin that the motor driver IN3 pin is connected to
int D1IN4pin = 7; // Arduino PWM pin that the motor driver ENA pin is connected to 

int D2ENA1pin = 14; // Arduino PWM pin that the motor driver ENA pin is connected to 
int D2IN1pin = 15; // Arduino pin that the motor driver IN1 pin is connected to
int D2IN2pin = 16; // Arduino pin that the motor driver IN2 pin is connected to

int D2ENA2pin = 17; // Change to a different PWM pin for D2
int D2IN3pin = 18; // Arduino pin that the motor driver IN3 pin is connected to
int D2IN4pin = 19; // Arduino PWM pin that the motor driver ENA pin is connected to 

int presentState;
int previousState;

unsigned long pulsesA = 0;

int pwm = 255;
int sec = 1000; //converts milliseconds to seconds in the delay function.

String incomingString = "";

void setup() {
  Serial1.begin(9600);
  while (!Serial);
  Serial1.println("Started");

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    ard_sensor_check = -1;
    while (1);
  }
  Serial.print("Gyroscope sample rate = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");
  Serial.println();
  Serial.println("Gyroscope in degrees/second");
  Serial.println("X\tY\tZ");

  // initiate (configure) Arduino pins as outputs
  pinMode(D1IN1pin, OUTPUT);
  pinMode(D1IN2pin, OUTPUT);
  pinMode(D1ENA1pin, OUTPUT);
  
  
  pinMode(D1IN3pin, OUTPUT);
  pinMode(D1IN4pin, OUTPUT);
  pinMode(D1ENA2pin, OUTPUT);

  pinMode(D2IN1pin, OUTPUT);
  pinMode(D2IN2pin, OUTPUT);
  pinMode(D2ENA1pin, OUTPUT);

  pinMode(D2IN3pin, OUTPUT);
  pinMode(D2IN4pin, OUTPUT);
  pinMode(D2ENA2pin, OUTPUT);

  // Initialize last state
  previousState = 0;
}

void loop() {

  switch(ard_sensor_check)
  {
    case -1:
      Serial.println("Something went seriously wrong? Check your wires and pray its not another faulty Arduino!");
    case 0:
      Serial.prntln("BRO What????");
    case 1:
      gyro_pulse();
      accel_pulse();
  }
}

void gyro_pulse(){
  float x,y,z;
  if(IMU.gyroscopeAvailable()){
    IMU.readGyroscope(x,y,z)
    ard_sensor_check = 1;
    Serial1.print(x);
    Serial1.print('\t');
    Serial1.print(y);
    Serial1.print('\t');
    Serial1.println(z);
  }
  else{
    ard_sensor_check = 0;
  }
}
void accel_pulse(){
  float x,y,z
  int degreesX;
  int degreesY;
  if(IMU.accelerationAvailable()){
    IMU.readAcceleration(x,y,z);
    if(x > 0.1){
      x = 100 * x;
      degreesX = map(x, 0, 97, 0, 90);
      Serial1.print("Tilting up ");
      Serial1.print(degreesX);
      Serial1.println(" degrees");
    }
    if(x < -0.1){
      x = 100 * x;
      degreesX = map(x, 0 , -100, 0, 90);
      Serial1.print("Tilting down ");
      Serial1.print(degreesX);
      Serial1.println(" degrees");
    }
    if(y > 0.1){
      y = 100 * y;
      degreesY = map(y, 0, 97, 0, 90);
      Serial1.print("Tilting left ");
      Serial1.print(degreesY);
      Serial1.println(" degrees");
    }
    if(y < -0.1){
      y = 100 * y;
      degreesY = map(y, 0, -100, 0, 90);
      Serial1.print("Tilting right ");
      Serial1.print(degreesY);
      Serial1.print(" degrees");
    }
  }
}

void uart_read(){
  if (Serial1.available() > 0){
    //This will read the uart data sent to it
    int inChar = Serial1.read();
    if (isDigit(inChar)){
      incomingString += (char)inChar;
    }
    if (inChar == '\n'){
      Serial.print("Value: ");
      Serial1.println(incomingString.toInt());
      Serial1.println(incomingString);
      incomingString = "";
    }
    switch(inChar)
      case 1:
        move_forward();
      case 2:
        move_backward();
      case 3:
        move_Strafe_Right();
      case 4:
        move_Strafe_Left();
      case 5:
        move_Diagonal_Top_Right();
      case 6:
        move_Diagonal_Top_Left();
      case 7:
        move_Diagonal_Bottom_Left();
      case 8:
        move_Diagonal_Bottom_Right();
      case 9:
        move_Rotate_Clockwise();
      case 10:
        move_Rotate_CounterClockwise();
  }
}

//Moves the robot forward
void move_forward()
{
  digitalWrite(D1IN1pin, HIGH);
  digitalWrite(D1IN2pin, LOW);
  analogWrite(D1ENA1pin, pwm * 0.25);

  digitalWrite(D1IN3pin, HIGH);
  digitalWrite(D1IN4pin, LOW);
  analogWrite(D1ENA2pin, pwm * 0.25);

  digitalWrite(D2IN1pin, HIGH);
  digitalWrite(D2IN2pin, LOW);
  analogWrite(D2ENA1pin, pwm * 0.25);

  digitalWrite(D2IN3pin, HIGH);
  digitalWrite(D2IN4pin, LOW);
  analogWrite(D2ENA2pin, pwm * 0.25);
}
//Moves backwards
void move_backward()
{
  digitalWrite(D1IN1pin, LOW);
  digitalWrite(D1IN2pin, HIGH);
  analogWrite(D1ENA1pin, pwm * 0.25);

  digitalWrite(D1IN3pin, LOW);
  digitalWrite(D1IN4pin, HIGH);
  analogWrite(D1ENA2pin, pwm * 0.25);

  digitalWrite(D2IN1pin, LOW);
  digitalWrite(D2IN2pin, HIGH);
  analogWrite(D2ENA1pin, pwm * 0.25);

  digitalWrite(D2IN3pin, LOW);
  digitalWrite(D2IN4pin, HIGH);
  analogWrite(D2ENA2pin, pwm * 0.25);
}

//This moves the robot to the right via strafe
void move_Strafe_Right(){
  digitalWrite(D1IN1pin, HIGH);
  digitalWrite(D1IN2pin, LOW);
  analogWrite(D1ENA1pin, pwm * 0.25);

  digitalWrite(D1IN3pin, LOW);
  digitalWrite(D1IN4pin, HIGH);
  analogWrite(D1ENA2pin, pwm * 0.25);

  digitalWrite(D2IN1pin, HIGH);
  digitalWrite(D2IN2pin, LOW);
  analogWrite(D2ENA1pin, pwm * 0.25);

  digitalWrite(D2IN3pin, LOW);
  digitalWrite(D2IN4pin, HIGH);
  analogWrite(D2ENA2pin, pwm * 0.25);
}
//This moves the robot to the left via strafe
void move_Strafe_Left(){
  digitalWrite(D1IN1pin, LOW);
  digitalWrite(D1IN2pin, HIGH);
  analogWrite(D1ENA1pin, pwm * 0.25);

  digitalWrite(D1IN3pin, HIGH);
  digitalWrite(D1IN4pin, LOW);
  analogWrite(D1ENA2pin, pwm * 0.25);

  digitalWrite(D2IN1pin, LOW);
  digitalWrite(D2IN2pin, HIGH);
  analogWrite(D2ENA1pin, pwm * 0.25);

  digitalWrite(D2IN3pin, HIGH);
  digitalWrite(D2IN4pin, LOW);
  analogWrite(D2ENA2pin, pwm * 0.25);
}
//This moves the robot diagonally top right
void move_Diagonal_Top_Right(){
  digitalWrite(D1IN1pin, HIGH);
  digitalWrite(D1IN2pin, LOW);
  analogWrite(D1ENA1pin, pwm * 0.25);

  digitalWrite(D1IN3pin, LOW);
  digitalWrite(D1IN4pin, HIGH);
  analogWrite(D1ENA2pin, pwm * 0);

  digitalWrite(D2IN1pin, HIGH);
  digitalWrite(D2IN2pin, LOW);
  analogWrite(D2ENA1pin, pwm * 0);

  digitalWrite(D2IN3pin, HIGH);
  digitalWrite(D2IN4pin, LOW);
  analogWrite(D2ENA2pin, pwm * 0.25);
}
//This moves the robot diagonally top left
void move_Diagonal_Top_Left(){
  digitalWrite(D1IN1pin, HIGH);
  digitalWrite(D1IN2pin, LOW);
  analogWrite(D1ENA1pin, pwm * 0);

  digitalWrite(D1IN3pin, HIGH);
  digitalWrite(D1IN4pin, LOW);
  analogWrite(D1ENA2pin, pwm * 0.25);

  digitalWrite(D2IN1pin, HIGH);
  digitalWrite(D2IN2pin, LOW);
  analogWrite(D2ENA1pin, pwm * 0.25);

  digitalWrite(D2IN3pin, LOW);
  digitalWrite(D2IN4pin, HIGH);
  analogWrite(D2ENA2pin, pwm * 0);
}
//This moves the robot diagonally bottom left
void move_Diagonal_Bottom_Left(){
  digitalWrite(D1IN1pin, HIGH);
  digitalWrite(D1IN2pin, LOW);
  analogWrite(D1ENA1pin, pwm * 0);

  digitalWrite(D1IN3pin, LOW);
  digitalWrite(D1IN4pin, HIGH);
  analogWrite(D1ENA2pin, pwm * 0.25);

  digitalWrite(D2IN1pin, HIGH);
  digitalWrite(D2IN2pin, LOW);
  analogWrite(D2ENA1pin, pwm * 0);

  digitalWrite(D2IN3pin, LOW);
  digitalWrite(D2IN4pin, HIGH);
  analogWrite(D2ENA2pin, pwm * 0.25);
  
}
//This moves the robot diagonally bottom right
void move_Diagonal_Bottom_Right(){
  digitalWrite(D1IN1pin, LOW);
  digitalWrite(D1IN2pin, HIGH);
  analogWrite(D1ENA1pin, pwm * 0.25);

  digitalWrite(D1IN3pin, HIGH);
  digitalWrite(D1IN4pin, LOW);
  analogWrite(D1ENA2pin, pwm * 0);

  digitalWrite(D2IN1pin, HIGH);
  digitalWrite(D2IN2pin, LOW);
  analogWrite(D2ENA1pin, pwm * 0);

  digitalWrite(D2IN3pin, LOW);
  digitalWrite(D2IN4pin, HIGH);
  analogWrite(D2ENA2pin, pwm * 0.25);
}
//Moves Clockwise
void move_Rotate_Clockwise(){
  digitalWrite(D1IN1pin, HIGH);
  digitalWrite(D1IN2pin, LOW);
  analogWrite(D1ENA1pin, pwm * 0.25);

  digitalWrite(D1IN3pin, HIGH);
  digitalWrite(D1IN4pin, LOW);
  analogWrite(D1ENA2pin, pwm * 0.25);

  digitalWrite(D2IN1pin, LOW);
  digitalWrite(D2IN2pin, HIGH);
  analogWrite(D2ENA1pin, pwm * 0.25);

  digitalWrite(D2IN3pin, LOW);
  digitalWrite(D2IN4pin, HIGH);
  analogWrite(D2ENA2pin, pwm * 0.25);
}
//Moves Counter Clockwise
void move_Rotate_CounterClockwise(){
  digitalWrite(D1IN1pin, LOW);
  digitalWrite(D1IN2pin, HIGH);
  analogWrite(D1ENA1pin, pwm * 0.25);

  digitalWrite(D1IN3pin, LOW);
  digitalWrite(D1IN4pin, HIGH);
  analogWrite(D1ENA2pin, pwm * 0.25);

  digitalWrite(D2IN1pin, HIGH);
  digitalWrite(D2IN2pin, LOW);
  analogWrite(D2ENA1pin, pwm * 0.25);

  digitalWrite(D2IN3pin, HIGH);
  digitalWrite(D2IN4pin, LOW);
  analogWrite(D2ENA2pin, pwm * 0.25);
}
//Turn on its rear axis
void move_Turn_RearAxis(){}
//Turn on its front axis
void move_Turn_FrontAxis(){}
