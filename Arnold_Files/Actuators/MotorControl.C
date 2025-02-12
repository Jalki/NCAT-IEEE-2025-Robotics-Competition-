/*
 * This code is a plain C version of an Arduino project.
 * It is intended as a template. You must supply the hardware‐dependent
 * implementations for the functions such as pinMode, digitalWrite,
 * digitalRead, delay_ms, and the I²C (TWI) functions.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// For AVR microcontrollers you might include these:
// #include <avr/io.h>
// #include <util/delay.h>

// ------------------------------
// Hardware abstraction stubs
// ------------------------------

// You will need to implement these functions based on your hardware.
// The following are stubs for simulation purposes.

#define OUTPUT 1
#define INPUT  0

// Stub: set a pin’s mode.
void pinMode(uint8_t pin, uint8_t mode) {
    // Implement hardware-specific pin mode setting here.
    // For example, on AVR you might modify DDR registers.
}

// Stub: write a digital value to a pin.
void digitalWrite(uint8_t pin, uint8_t value) {
    // Implement hardware-specific digital output here.
}

// Stub: read a digital value from a pin.
int digitalRead(uint8_t pin) {
    // Implement hardware-specific digital input here.
    // For simulation, you can return 0 or 1 as needed.
    return 0;
}

// Stub: delay in milliseconds.
// If using AVR, you might use _delay_ms(ms);
void delay_ms(int ms) {
    // Replace this with your platform’s delay function.
    // For example, on a POSIX system you might use:
    // usleep(ms * 1000);
    // Here we leave it as an empty stub.
}

// ------------------------------
// Pin definitions (adjust as needed)
// ------------------------------
enum {
    loaderLower = 2,
    loaderRaise = 3,
    brush       = 7,
    stepGeo     = 5,
    stepNeo     = 6,
    magnetWheel = 4,
    button      = 13,
    programmingSwitch = 12
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
void driveStepMotor(uint8_t motor, int duration);
void driveMagnetWheel(int duration);
void moveLoader(uint8_t dir, int duration);
void driveBrush(int duration);
void robotTest(void);
void rotateCW(int duration);
void rotateCCW(int duration);
void programmingMode(void);
void robotRoutine(void);
void readData(uint8_t data);  // Called from I²C receive (or interrupt) routine
void waitForMovement(int maxDelay);

// ------------------------------
// Main function
// ------------------------------
int main(void)
{
    init_system();
    while (1) {
        loop();
    }
    return 0;
}

// ------------------------------
// Initialization
// ------------------------------
void init_system(void)
{
    // Initialize I²C (TWI) with address 20 and set clock, and register a receive callback.
    // (Replace the following comments with your hardware-specific I²C initialization.)
    //
    // Example (pseudo-code):
    //   twi_init(20);
    //   twi_onReceive(readData);
    //   twi_setClock(10000);
    
    // Initialize Serial communication (baud rate 1000000) if needed.
    // Example (pseudo-code):
    //   uart_init(1000000);
    
    // Set pin modes for outputs.
    pinMode(loaderLower, OUTPUT);
    pinMode(loaderRaise, OUTPUT);
    pinMode(brush, OUTPUT);
    pinMode(stepGeo, OUTPUT);
    pinMode(stepNeo, OUTPUT);
    pinMode(magnetWheel, OUTPUT);
    
    delay_ms(200);
    
    // (Optional) Wait in programming mode if needed.
    // while(digitalRead(programmingSwitch) == HIGH)
    // {
    //    delay_ms(1000);
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
void sendData(int dir, int pulses)
{
    // Begin I²C transmission to device with address 21.
    // (Replace the following with your I²C routines.)
    //
    // Example (pseudo-code):
    //   twi_start(21);
    
    // For debugging, print the values.
    printf("%d %d %d\n", dir, (pulses & 65280) / 256, pulses & 255);
    
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
void driveStepMotor(uint8_t motor, int duration)
{
    digitalWrite(motor, 1);
    for (int i = 0; i < duration; i += 2) {
        delay_ms(1);
    }
    digitalWrite(motor, 0);
}

// ------------------------------
// Drive the magnet wheel
// ------------------------------
void driveMagnetWheel(int duration)
{
    digitalWrite(magnetWheel, 1);
    for (int i = 0; i < duration; i++) {
        delay_ms(1);
    }
    digitalWrite(magnetWheel, 0);
}

// ------------------------------
// Move the loader (up or down)
// ------------------------------
void moveLoader(uint8_t dir, int duration)
{
    digitalWrite(dir, 1);
    delay_ms(duration);
    digitalWrite(dir, 0);
}

// ------------------------------
// Drive the brush
// ------------------------------
void driveBrush(int duration)
{
    digitalWrite(brush, 1);
    delay_ms(duration);
    digitalWrite(brush, 0);
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
        delay_ms(500);
    }
    
    driveStepMotor(stepGeo, 4000);
    delay_ms(2000);
    driveStepMotor(stepNeo, 4000);
    delay_ms(2000);
    driveMagnetWheel(4000);
    delay_ms(2000);
    moveLoader(loaderRaise, 8000);
    delay_ms(2000);
    moveLoader(loaderLower, 8000);
    delay_ms(2000);
    driveBrush(4000);
    delay_ms(2000);
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
        delay_ms(1000);
        printf("programming mode\n");
    }
}

// ------------------------------
// Main robot routine
// ------------------------------
void robotRoutine(void)
{
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
void readData(uint8_t data)
{
    // In the original code, reading any data signifies that the
    // movement has finished.
    finishedMovement = true;
}

// ------------------------------
// Wait until movement is finished (or until timeout)
// ------------------------------
void waitForMovement(int maxDelay)
{
    while (!finishedMovement) {
        delay_ms(100);
        maxDelay -= 100;
        if (maxDelay <= 100) {
            finishedMovement = true;
        }
    }
    delay_ms(250);
    finishedMovement = false;
}
