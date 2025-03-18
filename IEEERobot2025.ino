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

const int photoPin = A0; // Analog pin connected to the photoresistor
int state = 0;          // 0 = calibration, 1 = start LED, 2 = normal state, 3 = cave state
float pde = 0.00;
float pde_h = 0.00;     // PDE +20% 
float pde_l = 0.00;     // PDE -20%
float pdel[2];          // Array for averages of pde_h and pde_l
float pdex = 0.00;      // PDE at the current time
float ptime = 0.00;       // Current time counter

unsigned long pulsesA = 0;

float positionX = 0, positionZ = 0, velocityX = 0, velocityZ = 0;
float accelDriftX = 0, accelDriftZ = 0;
float gyroDriftZ = 0;

#define ACCEL_NOISE_THRESHOLD 0.02  // Adjust based on testing
#define VELOCITY_DAMPING 0.98  // Reduces velocity gradually over time

int pwm = 255;
int sec = 1000; //converts milliseconds to seconds in the delay function.

String incomingString = "";

float positionalData[3] = {0.00, 0.00, 0.00};

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  while (!Serial);
  Serial1.println("Started Communication with Raspberry Pi");

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    ard_sensor_check = -1;
    while (1);
  }
  ard_sensor_check = 1;
  pinMode(photoPin, INPUT_PULLUP); // Set analog pin A0 as input

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
  control_movement();
  int sensorValue = analogRead(photoPin); // Read the analog value from the photoresistor
  float voltage = sensorValue * (5.0 / 1023.0); // Convert analog value to voltage
  float resistance = (voltage * 50000) / (5.0 - voltage); // Calculate resistance
  pde = (resistance / voltage); // PDE calculation

  //Serial.print("Voltage: ");
  //Serial.print(voltage);
  //Serial.print(" V, Resistance: ");
  //Serial.print(resistance);
  //Serial.println(" ohms");
  //Serial.print("Photoresistor Equivalent Data (PDE): ");
  //Serial.println(pde);
  //statemachine();
}

//Statemachine that correspond with the raspberry pi states to operate certain sensors
void statemachine(){
  if (Serial1.available()) {
        char state = Serial1.read();  // Read state

        Serial.print("Received State: ");
        Serial.println(state);

        switch (state) {
            case '0': 
              Serial.println("State 0 - Calibration");
              ALSAVG();
            case '1': 
              Serial.println("State 1 - Start Signal");
              PDEX();
            case '2': 
              Serial.println("State 2 - Normal Condition");
              PDEX();
              control_movement();
            case '3': 
              Serial.println("State 3 - Cave Condition");
              PDEX();
              control_movement();
            default: 
              Serial.println("Unknown State");
        }
    }
}

// Calculate room ambient light source average
void ALSAVG() {
  float pdel_h = pde * 1.20;
  float pdel_l = pde * 0.80;
  float pdel_harr[10];
  float pdel_larr[10];

  for (int i = 0; i < 10; i++) {
    pdel_harr[i] = pdel_h;
    pdel_larr[i] = pdel_l;
  }

  // Calculate average for pdel_harr
  float sum_h = 0.0;
  for (int i = 0; i < 10; i++) {
    sum_h += pdel_harr[i];
  }
  pde_h = sum_h / 10;

  // Calculate average for pdel_larr
  float sum_l = 0.0;
  for (int i = 0; i < 10; i++) {
    sum_l += pdel_larr[i];
  }
  pde_l = sum_l / 10;

  //Serial.print("Average pde_h: ");
  Serial1.println(pde_h);
  //Serial.print("Average pde_l: ");
  Serial1.println(pde_l);
} 

void PDEX() {
  pdex = pde; // Assign current pde to pdex
}

//Directly control the movement of the robot based on uart input from the raspberry pi. 
//Right now, the numbers for x and y needs to be relatively small, since our sensors produce a small number like 0.01, 0.04, etc
void control_movement() {
  float originData[3] = {0.00, 0.00, 0.00};
  //After setting the origin data, checks to make sure we are straight in position and angle!
  //pos_accel(0.00,0.00);
  //rotate_gyro(0.00);
  if (Serial1.available()){
    char input[50];
    int bytesRead = Serial1.readBytesUntil('\n', input, sizeof(input) -1);
    input[bytesRead] = '\0';
    float x, z, rotation;
    int parsed = sscanf(input, "%f,%f,%f", &x, &z, &rotation);

  if (parsed == 3){ //Ensure all three values were recieved
    Serial.print("Received X: ");
    Serial.println(x);
    Serial.print("Received Z: ");
    Serial.println(z);
    Serial.print("Recieved Rotation: ");
    Serial.println(rotation);
  }else{
    Serial.println("Data parse error!");
    }
  }
  delay(500);
}


//Plan script to navigate to the origins (0,0) of the robot
void navigate_to_origin() {
    float deltaX = -positionX; // Distance to move back to origin in X
    float deltaY = -positionZ; // Distance to move back to origin in Y

    if (deltaX > 0 && deltaY > 0) {
        move_Diagonal_Top_Right();
    } else if (deltaX < 0 && deltaY > 0) {
        move_Diagonal_Top_Left();
    } else if (deltaX < 0 && deltaY < 0) {
        move_Diagonal_Bottom_Left();
    } else if (deltaX > 0 && deltaY < 0) {
        move_Diagonal_Bottom_Right();
    } else if (deltaX > 0) {
        move_Strafe_Right();
    } else if (deltaX < 0) {
        move_Strafe_Left();
    } else if (deltaY > 0) {
        move_Forward();
    } else if (deltaY < 0) {
        move_Backward();
    }
    
    // Reset position after reaching origin
    positionX = 0;
    positionZ = 0;
    stop_movement();
}

//This script main function is to navigate to a set x and y position given by the raspberry pi via UART (look to the script above)
//Its in meters, but your numbers will be small, meters
void pos_accel(float targetDistanceX, float targetDistanceY) {
    float ax, ay, az;
    if (IMU.accelerationAvailable()) {
        IMU.readAcceleration(ax, ay, az);
        ax -= accelDriftX;
        az -= accelDriftZ;
        Serial.println(ax);
        Serial.println(az);
        if (abs(ax) < ACCEL_NOISE_THRESHOLD) ax = 0;
        if (abs(az) < ACCEL_NOISE_THRESHOLD) az = 0;
        velocityX = velocityX * VELOCITY_DAMPING + ax * 0.02;
        velocityZ = velocityZ * VELOCITY_DAMPING + az * 0.02;
        Serial.print(velocityX);
        Serial.print(velocityZ);
        positionX += velocityX * 0.02;
        positionZ += velocityZ * 0.02;
        Serial.print("Position: X = ");
        Serial.println(positionX);
        //Serial1.println(positionX);
        Serial.print(" Z = ");
        Serial1.println(positionZ);
        positionalData[0] = positionX;
        positionalData[1] = positionZ;
        //Serial1.print(positionY);
    }
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
    } else {
        move_Rotate_CounterClockwise();
    }

    // **3. Rotate while compensating for drift**
    while (abs(angleRotated) <= abs(targetAngle)) {  
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
            Serial1.println(angleRotated);
            positionalData[2] = angleRotated;
        }
    }
    stop_movement();  // Stop once the target angle is reached
}


//These functions is to rotate a certain degrees. Its -1 degree to accomodate the chassis!
void rotate_90_gyro() {
    rotate_gyro(89.0);
}

void rotate_180_gyro() {
    rotate_gyro(179.0);
}

void rotate_360_gyro() {
    rotate_gyro(359.0);
}

//Moves forward
void move_Forward() 
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
void move_Backward() 
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