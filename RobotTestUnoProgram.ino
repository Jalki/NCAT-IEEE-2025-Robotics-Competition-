#include <Wire.h>
int encoderPulses = 0;
int directionCommand;
int dataIn = 0;
int //output/input pins
  loaderLower = 2,
  loaderRaise = 3,
  brush = 7,
  stepGeo = 5,
  stepNeo = 6,
  magnetWheel = 4,
  button = 13,
  programmingSwitch = 12;

int //integer codes for directions
  stop = 0,
  forward = 1,
  backward = 2,
  right = 3,
  left = 4,
  upright = 5,
  upleft = 6,
  downright = 7,
  downleft = 8,
  CW = 9,
  CCW = 10;


 bool hasRun = false;
 bool finishedMovement = false; //boolean that is set to true when it receives data from arduino micro
 
  
void setup() {
  // put your setup code here, to run once:
  Wire.begin(20);
  Wire.onReceive(readData); //function to read I2C data
  Serial.begin(1000000);
  Wire.setClock(10000);
  pinMode(loaderLower, OUTPUT);
  pinMode(loaderRaise, OUTPUT);
  pinMode(brush, OUTPUT);
  pinMode(stepGeo, OUTPUT);
  pinMode(stepNeo, OUTPUT);
  pinMode(magnetWheel, OUTPUT);
  delay(200);
  /*while(digitalRead(programmingSwitch) == HIGH)
  {
    delay(1000);
    Serial.println("programmingMode");
    */
  }
  
  



void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(button) == HIGH) //if button is pressed the robot goes through a test of all its outputs
  {
    robotTest();
  }
  if (digitalRead(programmingSwitch) == HIGH) // if switch is in the up position the robot is in programming mode and will not initiate routine
  {
    programmingMode();
  }
  else if (hasRun == false) //only runs routine once
  {
    robotRoutine();
    hasRun = true;
  }
  //Serial.print(1);

  //48.7 pulses per inch
  //right = 3
  //666 = around 90 degrees
  
  //Serial.println(digitalRead(button));
  

  
  
  

}

void sendData(int dir, int pulses) //sends data to arduino micro
{
  Wire.beginTransmission(21); //arduino micro address is 21

  
  Serial.print(dir);
  Serial.print(" ");
  Serial.print((pulses & 65280)/256);
  Serial.print(" ");
  Serial.println(pulses & 255);
  
  Wire.write(dir); //sends data, first byte is direction, second byte is most siginificant byte for pulses, third byte is least significant byte for pulses
  Wire.write((pulses & 65280)/256);
  Wire.write(pulses & 255);
  Wire.endTransmission();
  
}
void driveStepMotor(int motor, int duration) //function to drive one of the 2 step motors
{
  digitalWrite(motor, HIGH);
  for (int i = 0; i < duration; i += 2)
  {
    
    delay(1);
  }
  digitalWrite(motor, LOW);
}
void driveMagnetWheel(int duration) //function to drive magnet wheel
{
  digitalWrite(magnetWheel, HIGH);
 
  for (int i = 0; i < duration; i ++)
  {
    delay(1);
    
  }
  digitalWrite(magnetWheel, LOW);
}
void moveLoader(int dir, int duration) // function to move loader up or down
{
  digitalWrite(dir, HIGH);
  delay(duration);
  digitalWrite(dir, LOW);
    
}
void driveBrush(int duration) // function to move brush on loader
{
  digitalWrite(brush, HIGH);
  delay(duration);
  digitalWrite(brush, LOW);
}

void robotTest() //runs through test of all outputs
{
  Serial.println("test routine"); 
  for(int i = 0; i < 11; i++ ) //runs through all movement commands
  {
    sendData(i, 40);
    waitForMovement(2000);
  
    delay(500);
    
  }
  
  
  
  driveStepMotor(stepGeo, 4000);
  delay(2000);
  driveStepMotor(stepNeo, 4000);
  delay(2000);
  driveMagnetWheel(4000);
  delay(2000);
  moveLoader(loaderRaise, 8000);
  delay(2000);
  moveLoader(loaderLower, 8000);
  delay(2000);
  driveBrush(4000);
  delay(2000);
}

void rotateCW(int duration) //rotates clockwise
{
  sendData(9, duration);
}

void rotateCCW(int dir, int duration) //rotates counterclockwise
{
  sendData(10, duration);
}

void programmingMode() //while switch is up arduino stays in programming mode
{
  sendData(stop, 0);
  Serial.println("test");
  while(digitalRead(programmingSwitch) == HIGH)
  {
    delay(1000);
    Serial.println("programming mode"); 
  }
}

void robotRoutine()
{

  //driveBrush(10000);

  
  //current robot routine

  moveLoader(loaderLower, 1000);
  sendData(forward, 1000);
  waitForMovement(10000);
  driveBrush(3000);
  sendData(backward, 150);
  moveLoader(loaderRaise, 8000);
  moveLoader(loaderLower, 8000);
  driveMagnetWheel(2000);
  sendData(backward, 75);
  waitForMovement(10000);
  sendData(right, 195);
  waitForMovement(10000);
  rotateCW(570);
  waitForMovement(5000);
  sendData(left, 800);
  waitForMovement(5000);
  sendData(right, 100);
  waitForMovement(5000);
  sendData(forward, 1266);
  waitForMovement(10000);
  sendData(right, 150);
  waitForMovement(10000);
  rotateCW(570);
  waitForMovement(10000);
  sendData(left, 600);
  waitForMovement(10000);
  sendData(right,375);
  waitForMovement(10000);
  sendData(forward, 950);
  waitForMovement(5000);
  driveStepMotor(stepGeo, 6000);
}
void readData(int bytes) //if I2C data is received it means the arduino micro has finished the movement
{
  Wire.read();
  finishedMovement = true;
}

void waitForMovement(int maxDelay) //function to wait until data is seen to indicate movement is finished
{
  while (!finishedMovement)
  {
    delay(100);
    maxDelay -= 100;
    if (maxDelay <= 100)
    {
      finishedMovement = true;
    }
  }
  delay(250);
  finishedMovement = false;
}
