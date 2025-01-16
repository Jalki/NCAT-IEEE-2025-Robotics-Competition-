#include <Wire.h>
int dataIn = 0;

//motor driver output pins
int rff = 8;  //right front forward
int rfr = 9; //right front reverse
int lff = 4; //left front forward
int lfr = 5; //left front rear
int rrf = 10; //right rear forward
int rrr = 16; //right rear reverse
int lrf = 6; //left rear forward
int lrr = 7; //left rear reverse

//encoder input pins
int encoderA = 15;
int encoderB = 14;

//tracks state of encoder outputs
bool presentStateA;
bool lastStateA;
bool presentStateB;
bool lastStateB;

//keeps track of number of pulses
unsigned long pulsesA;
unsigned long pulsesB;

//only runs robotRoutine function once
bool hasRun;

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
    
  Wire.begin(21); //Arduino micro address is 21
  Wire.onReceive(readData); //function to receive I2C data
  Serial.begin(1000000);
  for (int i = 0; i < 8; i++) //sets pin modes for outputs
  {
    pinMode(outputPins[i], OUTPUT);
  }

  pinMode(encoderA, INPUT);
  pinMode(encoderB, INPUT);


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

      Wire.beginTransmission(20); //begins transmission to main arduino
      Wire.write(1); //sends data to indicate the movement was successful, exact data is not critical at this point, data can be used for other purposes in the future
      Wire.endTransmission();
      Serial.println("done");
      



    }

  }



}

void readData(int bytes) //function to read I2C data
{
  dataIn = Wire.read(); // dataIn = direction command, number from 0 to 10
  directionCommand = dataIn;
  Serial.println(dataIn);
  pulsesByte0 = Wire.read(); //most significant byte of target pulses
  pulsesByte1 = Wire.read(); //least significant byte of target pulses
  Serial.println(pulsesByte0 * 255 + pulsesByte1); 
  hasSent = false;
  targetPulses = 0; //clears any remaining target pulses
  //Serial.println(pulsesByte0);
  //Serial.println(pulsesByte1);
  targetPulses += long(pulsesByte0) * 256; //most significant byte is multiplied by 256, which is 1 0000 0000 in binary or 100 in hexadecimal, first number after 1111 1111 or FF
  //Serial.println(targetPulses); 
  targetPulses += pulsesByte1; //least significant byte is just added to the total
  //Serial.println(targetPulses);
  pulsesA = 0; //resets pulse counts
  pulsesB = 0;
  //Serial.print(dataIn);
  //Serial.print(" ");
  //Serial.println(directionCommand);
  //Serial.print(" ");
  //Serial.println(targetPulses);

  switch (directionCommand) //translates integer direction comman into a string
  {
    case 0:
      currentCommand = "stop";
      break;
    case 1:
      currentCommand = "forward";
      break;
    case 2:
      currentCommand = "backward";
      break;
    case 3:
      currentCommand = "right";
      break;
    case 4:
      currentCommand = "left";
      break;
    case 5:
      currentCommand = "upright";
      break;
    case 6:
      currentCommand = "upleft";
      break;
    case 7:
      currentCommand = "downright";
      break;
    case 8:
      currentCommand = "downleft";
      break;
    case 9:
      currentCommand = "cw";
      break;
    case 10:
      currentCommand = "ccw";
      break;
  }
  sendCommand(currentCommand, targetPulses); //sendCommand function sets outputs
  hasSent = true;
}


void sendCommand(String dir, int pulse) 
{
  //Serial.println(dir);
  targetPulses = pulse;

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
