/*
 * This code is a plain C version of an Arduino project.
 * It is intended as a template. You must supply the hardware‐dependent
 * implementations for the functions such as pinModeset, digitalWrite,
 * digitalRead, delay, and the I²C (TWI) functions.
 */

 #include <stdio.h>
 #include <stdint.h>
 #include <stdbool.h>
 #include <wiringPi.h> // Include WiringPi library!
 
 // ------------------------------
 // Hardware abstraction stubs
 // ------------------------------
 
 // You will need to implement these functions based on your hardware.
 // The following are stubs for simulation purposes.
 
 #define OUTPUT 1
 #define INPUT  0
 
 
 void pinModeset(int pin, int mode) {
     // Implement hardware-specific pin mode setting here.
     // For example, on AVR you might modify DDR registers.
     pinMode(pin, OUTPUT);
   // pull up/down mode (PUD_OFF, PUD_UP, PUD_DOWN) => down
     pullUpDnControl(pin, mode);
 }
 
 
 
 // Stub: read a digital value from a pin.
 int digitalRead(int pin) {
     // Implement hardware-specific digital input here.
     // For simulation, you can return 0 or 1 as needed.
     return 0;
 }
 
 
 // ------------------------------
 // Pin definitions (adjust as needed)
 // ------------------------------
 enum {D7 = 10,//19
  D6 = 22,//15
    D5 = 27,//13
    D4 = 17,//11
    D3 = 4,//7
    D2 = 3,//5
    D1 = 2//3
 };


 enum {
     loaderLower = 113,//2 start encode 0001
     loaderRaise = 114,//3
     brush       = 115,//7
     stepGeo     = 116,//5
     stepNeo     = 117,//6
     magnetWheel = 118,//4
     button      = 119,//13
     programmingSwitch = 120//12
 };
 
 // ------------------------------
 // Direction codes
 // ------------------------------
 enum {
     stop      = 0,
     forward   = 1,
     backward  = 2,
     right     = 3,
     left      = 4,
     upright   = 5,
     upleft    = 6,
     downright = 7,
     downleft  = 8,
     CW        = 9,
     CCW       = 10
 };
 
 // ------------------------------
 // Global variables
 // ------------------------------
 volatile int encoderPulses = 0;
 volatile int directionCommand;
 volatile int dataIn = 0;
 
 volatile bool hasRun = false;
 volatile bool finishedMovement = false; // Indicates when movement is finished
 
 // ------------------------------
 // Function prototypes
 // ------------------------------
 void init_system(void);
 void loop(void);
 void sendData(int dir, int pulses);
 void driveStepMotor(int motor, int duration);
 void driveMagnetWheel(int duration);
 void moveLoader(int dir, int duration);
 void driveBrush(int duration);
 void robotTest(void);
 void rotateCW(int duration);
 void rotateCCW(int duration);
 void programmingMode(void);
 void robotRoutine(void);
 void readData(int data);  // Called from I²C receive (or interrupt) routine
 void waitForMovement(int maxDelay);
 void intTo4BitBinary(int num, int bits[6]);
 
 void digitalWriteDUT(int value) {
     // Dummy implementation of digitalWrite.
     // Uncomment the following line to print pin actions.
     // printf("Pin %d set to %d\n", pin, value);
	int binaryArray[7];    // Caller-provided array of length 4.
    intTo4BitBinary(value, binaryArray);
    digitalWrite(D7, binaryArray[0]);
    digitalWrite(D6, binaryArray[1]);
    digitalWrite(D5, binaryArray[2]);
    digitalWrite(D4, binaryArray[3]);
    digitalWrite(D3, binaryArray[4]);
    digitalWrite(D2, binaryArray[5]);
	digitalWrite(D1, binaryArray[6]);
 }
 
 // ------------------------------
 // Main function
 // ------------------------------
 int main(void)
 {
     init_system();
     while (1) {
      digitalWriteDUT(127);
      delay(3000);
      robotRoutine();
     }
     return 0;
 }
 
 // ------------------------------
 // Initialization
 // ------------------------------
 void init_system(void)
 {
    wiringPiSetupGpio();
 
     // Set pin modes for outputs.
     pinModeset(D1, OUTPUT);
     pinModeset(D2, OUTPUT);
     pinModeset(D3, OUTPUT);
     pinModeset(D4, OUTPUT);
     pinModeset(D5, OUTPUT);
     pinModeset(D6, OUTPUT);
     pinModeset(D7, OUTPUT);
     
     
     delay(200);
     
     // (Optional) Wait in programming mode if needed.
     // while(digitalRead(programmingSwitch) == HIGH)
     // {
     //    delay(1000);
     //    printf("programmingMode\n");
     // }
 }
 
 // ------------------------------
 // Main loop
 // ------------------------------
 void loop(void)
 {
     // If button is pressed, run test routine.
     if (digitalRead(button) == 1) {
         robotTest();
     }
     
     // If programming switch is up, enter programming mode.
     if (digitalRead(programmingSwitch) == 1) {
         programmingMode();
     }
     else if (!hasRun) {  // Only run the main routine once.
         robotRoutine();
         hasRun = true;
     }
 }
 
 // ------------------------------
 // Communication: send data via I²C
 // ------------------------------
 
 void intTo4BitBinary(int num, int bits[7]) {
     // Loop from the highest bit (n = 3, i.e., 2^3 = 8) to the lowest (n = 0, i.e., 2^0 = 1)
     for (int n = 6; n >= 0; n--) {
         int power = 1 << n;  // 2^n using bit-shift
         if (num >= power) {
             bits[6 - n] = 1; // Store bit; index 0 becomes the MSB, index 3 the LSB.
             num -= power;    // Subtract the power from num.
         } else {
             bits[6 - n] = 0;
         }
     }
 }
 
 
 void sendData(int dir, int pulses)
 {
     // Begin I²C transmission to device with address 21.
     // (Replace the following with your I²C routines.)
     //
     // Example (pseudo-code):
     //   twi_start(21);
     
     // For debugging, print the values.
     printf("%d %d %d\n", dir, (pulses & 65280) / 256, pulses & 255);

     digitalWriteDUT(dir);
     // Send the three bytes:
     //   twi_write(dir);
     //   twi_write((pulses & 65280) / 256);
     //   twi_write(pulses & 255);
     //
     // Finish transmission:
     //   twi_stop();
 }
 
 // ------------------------------
 // Drive one of the step motors
 // ------------------------------
 void driveStepMotor(int motor, int duration)
 {
     digitalWriteDUT(motor);
     for (int i = 0; i < duration; i += 2) {
         delay(1);
     }
     digitalWriteDUT(motor);
 }
 
 // ------------------------------
 // Drive the magnet wheel
 // ------------------------------
 void driveMagnetWheel(int duration)
 {
     digitalWriteDUT(magnetWheel);
     for (int i = 0; i < duration; i++) {
         delay(1);
     }
     digitalWriteDUT(magnetWheel);
 }
 
 // ------------------------------
 // Move the loader (up or down)
 // ------------------------------
 void moveLoader(int dir, int duration)
 {
     digitalWriteDUT(dir);
     delay(duration);
     digitalWriteDUT(dir);
 }
 
 // ------------------------------
 // Drive the brush
 // ------------------------------
 void driveBrush(int duration)
 {
     digitalWriteDUT(brush);
     delay(duration);
     digitalWriteDUT(brush);
 }
 
 // ------------------------------
 // Test routine: exercises all outputs
 // ------------------------------
 void robotTest(void)
 {
     printf("test routine\n");
     for (int i = 0; i < 11; i++ ) {
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
 
 // ------------------------------
 // Rotate clockwise
 // ------------------------------
 void rotateCW(int duration)
 {
     sendData(CW, duration);
 }
 
 // ------------------------------
 // Rotate counterclockwise
 // ------------------------------
 void rotateCCW(int duration)
 {
     sendData(CCW, duration);
 }
 
 // ------------------------------
 // Programming mode: remain idle while switch is up
 // ------------------------------
 void programmingMode(void)
 {
     sendData(stop, 0);
     printf("test\n");
     while (digitalRead(programmingSwitch) == 1) {
         delay(1000);
         printf("programming mode\n");
     }
 }
 
 // ------------------------------
 // Main robot routine
 // ------------------------------
 void robotRoutine(void)
 {
    digitalWriteDUT(31);
    delay(3000);
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
     sendData(right, 375);
     waitForMovement(10000);
     sendData(forward, 950);
     waitForMovement(5000);
     driveStepMotor(stepGeo, 6000);
 }
 
 // ------------------------------
 // I²C data received callback
 // ------------------------------
 void readData(int data)
 {
     // In the original code, reading any data signifies that the
     // movement has finished.
     while (true){
     finishedMovement = true;
     delay(5000);
    }
 }
 
 // ------------------------------
 // Wait until movement is finished (or until timeout)
 // ------------------------------
 void waitForMovement(int maxDelay){
     while (!finishedMovement) {
         delay(100);
         maxDelay -= 100;
         if (maxDelay <= 100) {
             finishedMovement = true;
         }
     }
     delay(250);
     finishedMovement = false;
 }
 //gcc -o W PWM.c  -l wiringPi