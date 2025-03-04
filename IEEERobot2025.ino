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

float positionX = 0, positionY = 0, velocityX = 0, velocityY = 0;
float accelDriftX = 0, accelDriftY = 0;
float gyroDriftZ = 0;

#define ACCEL_NOISE_THRESHOLD 0.02  // Adjust based on testing
#define VELOCITY_DAMPING 0.98  // Reduces velocity gradually over time

int pwm = 255;
int sec = 1000; //converts milliseconds to seconds in the delay function.

String incomingString = "";

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  while (!Serial);
  Serial1.println("Started");

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    ard_sensor_check = -1;
    while (1);
  }
  ard_sensor_check = 1;

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
  update_position();
}

void update_position() {
  control_movement();
}

//Directly control the movement of the robot based on uart input from the raspberry pi. 
void control_movement() {
  rotate_90_gyro();
}

//Moves forward
void move_forward()
{
  digitalWrite(D1IN1pin, HIGH);
  digitalWrite(D1IN2pin, LOW);
  analogWrite(D1ENA1pin, pwm * 0.75);

  digitalWrite(D1IN3pin, HIGH);
  digitalWrite(D1IN4pin, LOW);
  analogWrite(D1ENA2pin, pwm * 0.75);

  digitalWrite(D2IN1pin, HIGH);
  digitalWrite(D2IN2pin, LOW);
  analogWrite(D2ENA1pin, pwm * 0.75);

  digitalWrite(D2IN3pin, HIGH);
  digitalWrite(D2IN4pin, LOW);
  analogWrite(D2ENA2pin, pwm * 0.75);
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
void move_Strafe_Right()
{ 
  digitalWrite(D1IN1pin, HIGH);
  digitalWrite(D1IN2pin, LOW);
  analogWrite(D1ENA1pin, pwm * 0.25);

  digitalWrite(D1IN3pin, LOW);
  digitalWrite(D1IN4pin, HIGH);
  analogWrite(D1ENA2pin, pwm * 0.25);

  digitalWrite(D2IN1pin, LOW);
  digitalWrite(D2IN2pin, HIGH);
  analogWrite(D2ENA1pin, pwm * 0.25);

  digitalWrite(D2IN3pin, HIGH);
  digitalWrite(D2IN4pin, LOW);
  analogWrite(D2ENA2pin, pwm * 0.25);
}
//This moves the robot to the left via strafe
void move_Strafe_Left()
{ 
  digitalWrite(D1IN1pin, LOW);
  digitalWrite(D1IN2pin, HIGH);
  analogWrite(D1ENA1pin, pwm * 0.25);

  digitalWrite(D1IN3pin, HIGH);
  digitalWrite(D1IN4pin, LOW);
  analogWrite(D1ENA2pin, pwm * 0.25);

  digitalWrite(D2IN1pin, HIGH);
  digitalWrite(D2IN2pin, LOW);
  analogWrite(D2ENA1pin, pwm * 0.25);

  digitalWrite(D2IN3pin, LOW);
  digitalWrite(D2IN4pin, HIGH);
  analogWrite(D2ENA2pin, pwm * 0.25);
}
//This moves the robot diagonally top right
void move_Diagonal_Top_Right()
{ 
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
void move_Diagonal_Top_Left()
{ 
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
void move_Diagonal_Bottom_Left()
{
  digitalWrite(D1IN1pin, LOW);
  digitalWrite(D1IN2pin, HIGH);
  analogWrite(D1ENA1pin, pwm * 0.25);

  digitalWrite(D1IN3pin, LOW);
  digitalWrite(D1IN4pin, HIGH);
  analogWrite(D1ENA2pin, pwm * 0);

  digitalWrite(D2IN1pin, HIGH);
  digitalWrite(D2IN2pin, LOW);
  analogWrite(D2ENA1pin, pwm * 0);

  digitalWrite(D2IN3pin, LOW);
  digitalWrite(D2IN4pin, HIGH);
  analogWrite(D2ENA2pin, pwm * 0.25);
  
}
//This moves the robot diagonally bottom right
void move_Diagonal_Bottom_Right()
{ 
  digitalWrite(D1IN2pin, HIGH);
  analogWrite(D1ENA1pin, pwm * 0);

  digitalWrite(D1IN3pin, LOW);
  digitalWrite(D1IN4pin, HIGH);
  analogWrite(D1ENA2pin, pwm * 0.25);

  digitalWrite(D2IN1pin, LOW);
  digitalWrite(D2IN2pin, HIGH);
  analogWrite(D2ENA1pin, pwm * 0.25);

  digitalWrite(D2IN3pin, LOW);
  digitalWrite(D2IN4pin, HIGH);
  analogWrite(D2ENA2pin, pwm * 0);
}
//Moves Clockwise
void move_Rotate_Clockwise()
{
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
void move_Rotate_CounterClockwise()
{
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
void stop_movement()
{
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

void pos_accel(float targetDistance){
  float ax, ay, az;
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(ax, ay, az);
    ax -= accelDriftX;
    ay -= accelDriftY;
    // Ignore small values to reduce drift from sensor noise
    if (abs(ax) < ACCEL_NOISE_THRESHOLD) ax = 0;
    if (abs(ay) < ACCEL_NOISE_THRESHOLD) ay = 0;
    // Apply velocity damping before updating position
    velocityX = velocityX * VELOCITY_DAMPING + ax * 0.02;
    velocityY = velocityY * VELOCITY_DAMPING + ay * 0.02;
    velocityX += ax * 0.02;
    velocityY += ay * 0.02;
    positionX += velocityX * 0.02;
    positionY += velocityY * 0.02;
    Serial.print("Position of the robot: X: ");
    Serial.print(positionX);
    Serial.print(" Y: ");
    Serial.println(positionY);
  }
  stop_movement();
}

void rotate_gyro(float targetAngle) {
    float angleRotated = 0;
    float gx, gy, gz;
    float gyroDrift = 0;
    unsigned long prevTime = millis();

    // **1. Measure drift before rotation**  
    int numSamples = 50;
    for (int i = 0; i < numSamples; i++) {
        if (IMU.gyroscopeAvailable()) {
            IMU.readGyroscope(gx, gy, gz);
            gyroDrift += gz;  // Accumulate Z-axis drift
        }
        delay(10);  // Small delay between samples
    }
    gyroDrift /= numSamples;  // Average drift over samples

    // **2. Determine rotation direction**
    bool clockwise = (targetAngle > 0);
    if (clockwise) {
        move_Rotate_Clockwise();
    } else {_
        move_Rotate_CounterClockwise();
    }

    // **3. Rotate while compensating for drift**
    while (abs(angleRotated) < abs(targetAngle)) {  
        if (IMU.gyroscopeAvailable()) {
            IMU.readGyroscope(gx, gy, gz);
            

            // Apply drift correction
            gz -= gyroDrift;

            // Compute time elapsed
            unsigned long currentTime = millis();
            float deltaTime = (currentTime - prevTime) / 1000.00; // Convert ms to seconds
            prevTime = currentTime;

            // Integrate gyroscope data to estimate angle rotated
            angleRotated += gz * deltaTime;
        }
    }

    stop_movement();  // Stop once the target angle is reached
}


void rotate_90_gyro() {
    rotate_gyro(90);
}

void rotate_180_gyro() {
    rotate_gyro(180);
}

void rotate_360_gyro() {
    rotate_gyro(360);
}