#define MegaAddress 20
#define NanoAddress 21

#include <Wire.h>
#include "Arduino_BMI270_BMM150.h"

#include <stdlib.h> 

#include <math.h>
int interruptPin = 14;
int dataIn = 0;
String currentState;

float targetDistance;
float targetAngle;

float positionX = 0, positionZ = 0, velocityX = 0, velocityZ = 0, velocityY =0; //Variables to hold the data for the position of the arduino
float accelDriftX = 0, accelDriftZ = 0; // Variables to hold the drift of the accel
float gyroDriftZ = 0; //Variables to hold the drift of the gryoscope

#define ACCEL_NOISE_THRESHOLD 0.025  // Adjust based on testing
#define VELOCITY_DAMPING 0.98  // Reduces velocity gradually over time

//motor driver output pins
int rff = 7;  //right front forward
int rfr = 6; //right front reverse
int lff = 2; //left front forward
int lfr = 3; //left front reverse
int rrf = 9; //right rear forward
int rrr = 8; //right rear reverse
int lrf = 4; //left rear forward
int lrr = 5; //left rear reverse

//encoder input pins
int encoderA = 11;
int encoderB = 10;

//tracks state of encoder outputs
bool presentStateA;
bool lastStateA;
bool presentStateB;
bool lastStateB;

//keeps track of number of pulses
unsigned long pulsesA;
unsigned long pulsesB;

//only runs robotRoutine function once
bool hasRun = false;

//number of pulses until motors stop
unsigned long targetPulses;

//the number of pulses are received in two bytes, pulseByte0 is most significant byte
int pulsesByte0;
int pulsesByte1;

//determines the direction of movement
int directionCommand;

// array of output pins
int outputPins[] = { rff, rfr, lff, lfr, rrf, rrr, lrf, lrr };
int outputs[8]; // array to hold the outputs, determines state (HIGH or LOW) of output pins

//input array holds data for state of motors, the data is used to determine the correct state of the output pins
int input[4];

//current command determines state of motors
String currentCommand = "stop";

//hasSent is set to true when the state of the output pins is changed to the correct configuration
bool hasSent = false;


void setup() {
  // put your setup code here, to run once:
  pinMode(interruptPin, INPUT);
  
  //Serial.begin(1000000);
  for (int i = 0; i < 8; i++) //sets pin modes for outputs
  {
    pinMode(outputPins[i], OUTPUT);
  }

  pinMode(encoderA, INPUT);
  pinMode(encoderB, INPUT);
  delay(1000);
  IMU.begin();

  Wire.begin(NanoAddress); //Arduino nano address is 21
  Wire.onReceive(readData); //function to receive I2C data

  
  


}

void loop() {
  // put your main code here, to run repeatedly:
  
 
  
  /* if (hasSent == false)
  {
    sendCommand(currentCommand, targetPulses);
    hasSent = true;
    //Serial.println("test");
  }
  */
  
  /*
  if (targetPulses > 0) // target pulses has to be greater than 0 to activate motors
  {
    //tracks state of encoders
    presentStateA = digitalRead(encoderA); 
    presentStateB = digitalRead(encoderB);

    //Serial.print(presentState);
    //Serial.println(lastState);

    if (presentStateA != lastStateA) //if pulse transition occured:
    {

      if (presentStateA) //if transition was from low to high the pulse count is increased
      {
        pulsesA ++;
      }

      //Serial.println("test");

      lastStateA = presentStateA; //always sets lastState to the current state
    }
    if (presentStateB != lastStateB) {

      if (presentStateB)
      {
        pulsesB ++;
      }

      //Serial.println("test");

      lastStateB = presentStateB;
    }
    //both encoders are tracked, as soon as one encoder reaches the target number of pulses the motors are stopped
    
    
     // encoders not used currently
    Serial.print("target pulses: ");
    Serial.println(targetPulses);
    Serial.print(pulsesB);
    Serial.print(" ");
    Serial.println(pulsesA);
    if ((targetPulses <= pulsesB || targetPulses <= pulsesA) && currentCommand != "stop" || (currentCommand == "stop" && targetPulses > 0))//if target pulses have been exceeded by the pulse count or if the current command is stop but there are target pulses remaining
    {
      Serial.println(currentCommand);
      currentCommand = "stop"; //sets current command to stop
      targetPulses = 0; //sets target pulses to 0
      
      //Serial.print(pulsesA);
      //Serial.print(pulsesB);
      //Serial.println(targetPulses);
      sendCommand("stop", 0); //sets outputs to correct confiuration

     
      Serial.println("done");
      
      



    }

  }
  */



}

void readData(int bytes) //function to read I2C data
{
  int dataByte0;
  int dataByte1;
  dataIn = Wire.read(); // dataIn = direction command, number from 0 to 10
  directionCommand = dataIn;
  //Serial.println(dataIn);
  
  dataByte0 = Wire.read(); //most significant byte of data
  dataByte1 = Wire.read(); //least significant byte of data

  float data = dataByte0 * 256 + dataByte1;

  float distanceScaleFactor = 1000.0;

  float angleScaleFactor = 10.0;


  switch (directionCommand) // translates integer direction command into a string
  {
    case 0:
      stop_movement();
      break;
    case 1:
      moveForward(data/distanceScaleFactor);
      break;
    case 2:
      moveBackward(data/distanceScaleFactor);
      break;
    case 3:
      moveRight(data/distanceScaleFactor);
      break;
    case 4:
      moveLeft(data/distanceScaleFactor);
      break;
    case 5:
      currentCommand = "upright";
      sendCommand(currentCommand, 100); //sendCommand function sets outputs
      break;
    case 6:
      currentCommand = "upleft";
      sendCommand(currentCommand, 100); //sendCommand function sets outputs
      break;
    case 7:
      currentCommand = "downright";
      sendCommand(currentCommand, 100); //sendCommand function sets outputs
      break;
    case 8:
      currentCommand = "downleft";
      sendCommand(currentCommand, 100); //sendCommand function sets outputs
      break;
    case 9:
      rotate_gyro(data/angleScaleFactor);
      break;
    case 10:
      rotate_gyro(-(data/angleScaleFactor));
      break;
  }
  
  hasSent = true;
  
}


void sendCommand(String dir, int pulse) 
{
  //Serial.println(dir);
  targetPulses = pulse;
  currentState = dir;

  //sets state of motors
  //0 = stopped, 1 = forward, 2 = backward
  if (dir == "forward") 
  {
    input[0] = 1;
    input[1] = 1;
    input[2] = 1;
    input[3] = 1;
  }
  if (dir == "backward") 
  {
    input[0] = 2;
    input[1] = 2;
    input[2] = 2;
    input[3] = 2;
  }
  if (dir == "right") 
  {
    input[0] = 2;
    input[1] = 1;
    input[2] = 1;
    input[3] = 2;
  }
  if (dir == "left") 
  {
    input[0] = 1;
    input[1] = 2;
    input[2] = 2;
    input[3] = 1;
  }
  if (dir == "upright") 
  {
    input[0] = 0;
    input[1] = 1;
    input[2] = 1;
    input[3] = 0;
  }
  if (dir == "upleft") 
  {
    input[0] = 1;
    input[1] = 0;
    input[2] = 0;
    input[3] = 1;
  }
  if (dir == "downright") 
  {
    input[0] = 2;
    input[1] = 0;
    input[2] = 0;
    input[3] = 2;
  }
  if (dir == "downleft") 
  {
    input[0] = 0;
    input[1] = 2;
    input[2] = 2;
    input[3] = 0;
  }
  if (dir == "cw") 
  {
    input[0] = 2;
    input[1] = 1;
    input[2] = 2;
    input[3] = 1;
  }
  if (dir == "ccw") 
  {
    input[0] = 1;
    input[1] = 2;
    input[2] = 1;
    input[3] = 2;
  }
  if (dir == "stop") 
  {
    input[0] = 0;
    input[1] = 0;
    input[2] = 0;
    input[3] = 0;
  }
  copyOuts(); //copyOuts function uses input array to set state of output pins
}

void copyOuts() //sets state of output pins
{
  for (int i = 0; i < 4; i++) // loops through input array
  {
    if (input[i] == 0) //if motor is off (0), both output pins are off
    { 
      outputs[2 * i] = 0;
      outputs[2 * i + 1] = 0;
    }
    if (input[i] == 1) //if motor is forward (1), the forward pin is turned on and the reverse pin is turned off
    {
      outputs[2 * i] = 1;
      outputs[2 * i + 1] = 0;
    }
    if (input[i] == 2) //if motor is reverse (2), the forward pin is turned off and the reverse pin is turned on
    {
      outputs[2 * i] = 0;
      outputs[2 * i + 1] = 1;
    }
  }

  for (int i = 0; i < 8; i++) 
  {
    digitalWrite(outputPins[i], LOW);
  }
  for (int i = 0; i < 8; i++) 
  {
    digitalWrite(outputPins[i], outputs[i]);
    //Serial.print(outputPins[i]);
    //Serial.println(outputs[i]);
  }
}

/*void reportPulses() // not used anymore
{
  
      Wire.beginTransmission(20); //begins transmission to main arduino 
      //Wire.write(byte (pulsesA / 256));
      Wire.write(1);
      //Wire.write(byte (pulsesA & 255)); //sends data to indicate the movement was successful, exact data is not critical at this point, data can be used for other purposes in the future
      Wire.endTransmission();
      pulsesA = 0; //resets pulse counts
      pulsesB = 0;
}
*/

void rotate_gyro(float targetAngleIn) {
    targetAngle = targetAngleIn;
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
            //Serial.println(gz);
        }
        delay(10);  // Small delay between samples
    }
    gyroDrift /= numSamples;  // Average drift over samples
    //Serial.println("e");

    // **2. Determine rotation direction**
    bool clockwise = (targetAngle > 0);
    if (clockwise) {
        move_Rotate_Clockwise();
    } else {
        move_Rotate_CounterClockwise();
    }

    // **3. Rotate while compensating for drift**
    while (abs(angleRotated) <= abs(targetAngle)  && digitalRead(A4) && digitalRead(A5)) {  
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
           // Serial.println(angleRotated);
            //positionalData[2] = angleRotated;
        }
    }
    stop_movement();  // Stop once the target angle is reached
}

void  stop_movement()
{
  sendCommand("stop", 0);
  targetDistance = 0.0;
  targetAngle = 0.0;
}

void move_Rotate_Clockwise()
{
  sendCommand("cw", 100);
  
}

void move_Rotate_CounterClockwise()
{
  sendCommand("ccw", 100);
  
}

void  moveForward(float targetDistanceIn)
{
  sendCommand("forward", 0);
  float deltaTime = 0.0;
  unsigned long prevTime = 0;
  velocityY = 0;
  velocityX = 0;
  targetDistance = targetDistanceIn;  
  while(targetDistance > 0 && currentState == "forward" && !Wire.available() && (digitalRead(A4)) && digitalRead(A5))
  {
  
    float ax, ay, az;
    //float dpos_x, dpos_z;
   
    if (IMU.accelerationAvailable()) 
    {
        IMU.readAcceleration(ax, ay, az);
        ax -= accelDriftX;
        az -= accelDriftZ;
        //Serial.print("y_accel:");
       // Serial.print(ay);
        //Serial.print(",");
        //Serial.print("x_accel:");
        //Serial.print(ax);
        //Serial.print(",");
        //Serial.print("vel_x");
        //Serial.print(velocityX);
       // Serial.print(",");
        //Serial.print("TargetDistance");
       // Serial.println(targetDistance);
        if (abs(ax) < ACCEL_NOISE_THRESHOLD) ax = 0;
        //if (abs(az) < ACCEL_NOISE_THRESHOLD) az = 0;
        //if (abs(ay) < ACCEL_NOISE_THRESHOLD) ay = 0;
        ax *= 9.8;    // values are given in terms of g, scale by 9.8 for m/s^2
        //velocityX = velocityX * VELOCITY_DAMPING + ax * deltaTime;
        velocityX = velocityX * VELOCITY_DAMPING + ax * deltaTime;
        //Serial.print(velocityX);
        //Serial.print(velocityZ);
        //positionX += velocityX * 0.02;
        targetDistance -= abs(velocityX) * deltaTime;
        //Serial.print("Position: X = ");
        //Serial.println(positionX);
        //Serial1.println(positionX);
        //Serial.print(" Z = ");
        //Serial1.print(positionY);
        
        unsigned long currentTime = millis();
        if (prevTime == 0)
        {
          prevTime = currentTime;
        }
        deltaTime = (currentTime - prevTime) / 1000.00; // Convert ms to seconds
        prevTime = currentTime;
    }
        //Only to use it for the d 
        
      
  }
  stop_movement();
}

void  moveBackward(float targetDistanceIn)
{
  sendCommand("backward", 0);
  float deltaTime = 0.0;
  unsigned long prevTime = 0;
   velocityY = 0;
    velocityX = 0;
  targetDistance = targetDistanceIn;  
  while(targetDistance > 0 && (digitalRead(interruptPin) == LOW) && digitalRead(A4) && digitalRead(A5))
  {
  
    float ax, ay, az;
    //float dpos_x, dpos_z;
   
    if (IMU.accelerationAvailable()) 
    {
        IMU.readAcceleration(ax, ay, az);
        ax -= accelDriftX;
        az -= accelDriftZ;

        if (abs(ax) < ACCEL_NOISE_THRESHOLD) ax = 0;
        ax *= 9.8;    // values are given in terms of g, scale by 9.8 for m/s^2
       
        velocityX = velocityX * VELOCITY_DAMPING + ax * deltaTime;
        
        targetDistance -= abs(velocityX) * deltaTime;
        
        unsigned long currentTime = millis();
        if (prevTime == 0)
        {
          prevTime = currentTime;
        }
        deltaTime = (currentTime - prevTime) / 1000.00; // Convert ms to seconds
        prevTime = currentTime;
    }
        //Only to use it for the d 
        
      
  }
  stop_movement();
}

void  moveLeft(float targetDistanceIn)
{
  sendCommand("left", 0);
  float deltaTime = 0.0;
  unsigned long prevTime = 0;
   velocityY = 0;
    velocityZ = 0;
  targetDistance = targetDistanceIn;   
  while(targetDistance > 0 && (digitalRead(interruptPin) == LOW) && digitalRead(A4) && digitalRead(A5))
  {
  
    float ax, ay, az;
    //float dpos_x, dpos_z;
   
    if (IMU.accelerationAvailable()) 
    {
        IMU.readAcceleration(ax, ay, az);
        ax -= accelDriftX;
        ay -= accelDriftX;

        if (abs(ay) < ACCEL_NOISE_THRESHOLD) ay = 0;
        ay *= 9.8;    // values are given in terms of g, scale by 9.8 for m/s^2
       
        velocityY = velocityY * VELOCITY_DAMPING + ay * deltaTime;
        
        targetDistance -= abs(velocityY) * deltaTime;
        
        unsigned long currentTime = millis();
        if (prevTime == 0)
        {
          prevTime = currentTime;
        }
        deltaTime = (currentTime - prevTime) / 1000.00; // Convert ms to seconds
        prevTime = currentTime;
    }
        //Only to use it for the d 
        
      
  }
  stop_movement();
}

void  moveRight(float targetDistanceIn)
{
  sendCommand("right", 0);
  float deltaTime = 0.0;
  unsigned long prevTime = 0;
   velocityY = 0;
    velocityZ = 0;
  targetDistance = targetDistanceIn;   
  while(targetDistance > 0 && (digitalRead(interruptPin) == LOW) && digitalRead(A4) && digitalRead(A5))
  {
  
    float ax, ay, az;
    //float dpos_x, dpos_z;
   
    if (IMU.accelerationAvailable()) 
    {
        IMU.readAcceleration(ax, ay, az);
        ax -= accelDriftX;
        ay -= accelDriftX;

        if (abs(ay) < ACCEL_NOISE_THRESHOLD) ay = 0;
        ay *= 9.8;    // values are given in terms of g, scale by 9.8 for m/s^2
       
        velocityY = velocityY * VELOCITY_DAMPING + ay * deltaTime;
        
        targetDistance -= abs(velocityY) * deltaTime;
        
        unsigned long currentTime = millis();
        if (prevTime == 0)
        {
          prevTime = currentTime;
        }
        deltaTime = (currentTime - prevTime) / 1000.00; // Convert ms to seconds
        prevTime = currentTime;
    }
        //Only to use it for the d 
        
      
  }
  stop_movement();
}
