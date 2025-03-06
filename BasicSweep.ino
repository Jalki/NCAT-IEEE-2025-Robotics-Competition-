int ard_sensor_check; //This is to make sure everything goes well with sensors

#include "Arduino_BMI270_BMM150.h"

#include <Wire.h>
#include <stdlib.h> 

//D1N1 & D1N2 = Top Left Wheel
//D1N3 & D1N4 = Top Right Wheel
//D2N1 & D2N2 = Bottom Left Wheel
//D2N3 & D2N4 = Bottom Right Wheel

//Modification
//D1N1 & D1N2 = Top Right Wheel
//D1N3 & D1N4 = Bottom Right Wheel
//D2N1 & D2N2 = Bottom Left Wheel
//D2N3 & D2N4 = Top Left Wheel


/**
 * D = Driver
 * IN = Incoder
 * EMA = Enable(PWM)
 * 
 * 
 */
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

//Moves forward
void move_forward() 
{
  digitalWrite(D1IN1pin, LOW);
  digitalWrite(D1IN2pin, HIGH);
  analogWrite(D1ENA1pin, pwm * 0.50);

  digitalWrite(D1IN3pin, LOW);
  digitalWrite(D1IN4pin, HIGH);
  analogWrite(D1ENA2pin, pwm * 0.50);

  digitalWrite(D2IN1pin, LOW);
  digitalWrite(D2IN2pin, HIGH);
  analogWrite(D2ENA1pin, pwm * 0.50);

  digitalWrite(D2IN3pin, LOW);
  digitalWrite(D2IN4pin, HIGH);
  analogWrite(D2ENA2pin, pwm * 0.50);
}

//Moves backwards
void move_backward() 
{
  digitalWrite(D1IN1pin, HIGH);
  digitalWrite(D1IN2pin, LOW);
  analogWrite(D1ENA1pin, pwm * 0.50);

  digitalWrite(D1IN3pin, HIGH);
  digitalWrite(D1IN4pin, LOW);
  analogWrite(D1ENA2pin, pwm * 0.50);

  digitalWrite(D2IN1pin, HIGH);
  digitalWrite(D2IN2pin, LOW);
  analogWrite(D2ENA1pin, pwm * 0.50);

  digitalWrite(D2IN3pin, HIGH);
  digitalWrite(D2IN4pin, LOW);
  analogWrite(D2ENA2pin, pwm * 0.50);
}

//This moves the robot to the right via strafe
void move_Strafe_Right(){ 
  digitalWrite(D1IN1pin, LOW);
  digitalWrite(D1IN2pin, HIGH);
  analogWrite(D1ENA1pin, pwm * 0.5);

  digitalWrite(D1IN3pin, HIGH);
  digitalWrite(D1IN4pin, LOW);
  analogWrite(D1ENA2pin, pwm * 0.5);

  digitalWrite(D2IN1pin, LOW);
  digitalWrite(D2IN2pin, HIGH);
  analogWrite(D2ENA1pin, pwm * 0.5);

  digitalWrite(D2IN3pin, HIGH);
  digitalWrite(D2IN4pin, LOW);
  analogWrite(D2ENA2pin, pwm * 0.5);
}
//This moves the robot to the left via strafe
void move_Strafe_Left(){ 
  digitalWrite(D1IN1pin, HIGH);
  digitalWrite(D1IN2pin, LOW);
  analogWrite(D1ENA1pin, pwm * 0.50);

  digitalWrite(D1IN3pin, LOW);
  digitalWrite(D1IN4pin, HIGH);
  analogWrite(D1ENA2pin, pwm * 0.50);

  digitalWrite(D2IN1pin, HIGH);
  digitalWrite(D2IN2pin, LOW);
  analogWrite(D2ENA1pin, pwm * 0.50);

  digitalWrite(D2IN3pin, LOW);
  digitalWrite(D2IN4pin, HIGH);
  analogWrite(D2ENA2pin, pwm * 0.50);
}
//This moves the robot diagonally top right
void move_Diagonal_Top_Right(){
  digitalWrite(D1IN1pin, HIGH);
  digitalWrite(D1IN2pin, LOW);
  analogWrite(D1ENA1pin, pwm * 0);

  digitalWrite(D1IN3pin, LOW);
  digitalWrite(D1IN4pin, HIGH);
  analogWrite(D1ENA2pin, pwm * 0.5);

  digitalWrite(D2IN1pin, HIGH);
  digitalWrite(D2IN2pin, LOW);
  analogWrite(D2ENA1pin, pwm * 0);

  digitalWrite(D2IN3pin, LOW);
  digitalWrite(D2IN4pin, HIGH);
  analogWrite(D2ENA2pin, pwm * 0.5);
}
//This moves the robot diagonally top left
void move_Diagonal_Top_Left(){ 
  digitalWrite(D1IN1pin, LOW);
  digitalWrite(D1IN2pin, HIGH);
  analogWrite(D1ENA1pin, pwm * 0.5);

  digitalWrite(D1IN3pin, HIGH);
  digitalWrite(D1IN4pin, LOW);
  analogWrite(D1ENA2pin, pwm * 0);

  digitalWrite(D2IN1pin, LOW);
  digitalWrite(D2IN2pin, HIGH);
  analogWrite(D2ENA1pin, pwm * 0.5);

  digitalWrite(D2IN3pin, LOW);
  digitalWrite(D2IN4pin, HIGH);
  analogWrite(D2ENA2pin, pwm * 0);
}
//This moves the robot diagonally bottom left
void move_Diagonal_Bottom_Left(){ 
  digitalWrite(D1IN1pin, HIGH);
  digitalWrite(D1IN2pin, LOW);
  analogWrite(D1ENA1pin, pwm * 0.5);

  digitalWrite(D1IN3pin, HIGH);
  digitalWrite(D1IN4pin, LOW);
  analogWrite(D1ENA2pin, pwm * 0);

  digitalWrite(D2IN1pin, HIGH);
  digitalWrite(D2IN2pin, LOW);
  analogWrite(D2ENA1pin, pwm * 0.5);

  digitalWrite(D2IN3pin, LOW);
  digitalWrite(D2IN4pin, HIGH);
  analogWrite(D2ENA2pin, pwm * 0);
  
}
//This moves the robot diagonally bottom right
void move_Diagonal_Bottom_Right(){ 
  digitalWrite(D1IN1pin, HIGH);
  digitalWrite(D1IN2pin, LOW);
  analogWrite(D1ENA1pin, pwm * 0);

  digitalWrite(D1IN3pin, HIGH);
  digitalWrite(D1IN4pin, LOW);
  analogWrite(D1ENA2pin, pwm * 0.5);

  digitalWrite(D2IN1pin, HIGH);
  digitalWrite(D2IN2pin, LOW);
  analogWrite(D2ENA1pin, pwm * 0);

  digitalWrite(D2IN3pin, HIGH);
  digitalWrite(D2IN4pin, LOW);
  analogWrite(D2ENA2pin, pwm * 0.5);
}
//Moves Clockwise
void move_Rotate_Clockwise(){
  digitalWrite(D1IN1pin, HIGH);
  digitalWrite(D1IN2pin, LOW);
  analogWrite(D1ENA1pin, pwm * 0.50);

  digitalWrite(D1IN3pin, HIGH);
  digitalWrite(D1IN4pin, LOW);
  analogWrite(D1ENA2pin, pwm * 0.50);

  digitalWrite(D2IN1pin, LOW);
  digitalWrite(D2IN2pin, HIGH);
  analogWrite(D2ENA1pin, pwm * 0.50);

  digitalWrite(D2IN3pin, LOW);
  digitalWrite(D2IN4pin, HIGH);
  analogWrite(D2ENA2pin, pwm * 0.50);
}
//Moves Counter Clockwise
void move_Rotate_CounterClockwise(){  
  digitalWrite(D1IN1pin, LOW);
  digitalWrite(D1IN2pin, HIGH);
  analogWrite(D1ENA1pin, pwm * 0.50);

  digitalWrite(D1IN3pin, LOW);
  digitalWrite(D1IN4pin, HIGH);
  analogWrite(D1ENA2pin, pwm * 0.50);

  digitalWrite(D2IN1pin, HIGH);
  digitalWrite(D2IN2pin, LOW);
  analogWrite(D2ENA1pin, pwm * 0.50);

  digitalWrite(D2IN3pin, HIGH);
  digitalWrite(D2IN4pin, LOW);
  analogWrite(D2ENA2pin, pwm * 0.50);
}

// Function to stop all motors
void stop_movement() {
  digitalWrite(D1IN1pin, LOW);
  digitalWrite(D1IN2pin, LOW);
  digitalWrite(D1IN3pin, LOW);
  digitalWrite(D1IN4pin, LOW);
  digitalWrite(D2IN1pin, LOW);
  digitalWrite(D2IN2pin, LOW);
  digitalWrite(D2IN3pin, LOW);
  digitalWrite(D2IN4pin, LOW);
  analogWrite(D1ENA1pin, 0);
  analogWrite(D1ENA2pin, 0);
  analogWrite(D2ENA1pin, 0);
  analogWrite(D2ENA2pin, 0);
}



void setup(){
  Serial.begin(9600);
}

void loop(){
//  Test all moving functions 5 second apart
//  move_forward();
//  delay(3000);
//  move_backward();
//  delay(3000);
//  move_Strafe_Right();
//  delay(3000);
//  move_Strafe_Left();
//  delay(3000);
//  move_Diagonal_Top_Right();
//  delay(3000);
//  move_Diagonal_Top_Left();
//  delay(3000);
//  move_Diagonal_Bottom_Right();
//  delay(3000);
//  move_Diagonal_Bottom_Left();
//  delay(3000);
//  move_Rotate_Clockwise();
//  delay(3000);
//  move_Rotate_CounterClockwise();
//  delay(3000);
//  stop_movement();
//  delay(10000);

//Test how long to run from start position to the wall across it
//  Serial.println("I'm moving forward");
//  move_forward();
//  delay(5000);
//  Serial.println("I'm not moving");
//  //stop_movement();
//  //delay(5000);

//Test how long it takes for robot to rotate 90 degrees
//   move_Rotate_Clockwise();

//Sweeping Track Algorithm
  move_forward();
  delay(3200);
  move_Rotate_Clockwise();
  delay(1290);
  move_forward();
  delay(500);
  move_Rotate_Clockwise();
  delay(1290);
  move_forward();
  delay(3200);
  move_Rotate_CounterClockwise();
  delay(1290);
  move_forward();
  delay(500);
  move_Rotate_CounterClockwise();
  delay(1290);
  move_forward();
  delay(3200);
  stop_movement();
  delay(10000);
   
  
}
