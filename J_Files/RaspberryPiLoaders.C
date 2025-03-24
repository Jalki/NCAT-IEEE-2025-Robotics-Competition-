#include <stdio.h>    // Standard I/O functions
#include <stdlib.h>   // Standard C library
#include <wiringPi.h> // Wiring Pi
#include <fcntl.h>    // File control
#include <unistd.h>   // POSIX API (sleep, read, write)
#include <termios.h>  // Terminal I/O
#include <string.h>   // String functions
#include <errno.h>    // Error handling
#include <sys/time.h> // System time functions
#include <softPwm.h>

// Define BCM pins for the motors
#define Brush_Motor 23 // BCM Pin 23
#define Step_Motor 24  // BCM Pin 24
#define Screw_Motor 25 // BCM Pin 25
#define Loader_Raise 8 // BCM Pin 08
#define Loader_Lower 7 // BCM Pin 07

// State variables to track motor status
int Brush_Motor_State = 0;
int Step_Motor_State = 0;
int Screw_Motor_State = 0;
int Loader_Raise_State = 0;
int Loader_Lower_State = 0;

void setup()
{
    // Set motor control pins to output
    pinMode(Brush_Motor, OUTPUT);
    pinMode(Step_Motor, OUTPUT);
    pinMode(Screw_Motor, OUTPUT);
    pinMode(Loader_Lower, OUTPUT);
    pinMode(Loader_Raise, OUTPUT);

    // Initialize all motors to OFF state
    digitalWrite(Brush_Motor, LOW);
    digitalWrite(Step_Motor, LOW);
    digitalWrite(Screw_Motor, LOW);
    digitalWrite(Loader_Lower, LOW);
    digitalWrite(Loader_Raise, LOW);
}

// Function to turn the Brush motor ON
void BrushOn() {
    if (Brush_Motor_State == 0) { // Only turn on if currently off
        printf("Brush motor ON\n");
        digitalWrite(Brush_Motor, HIGH); // Turn on motor
        Brush_Motor_State = 1; // Update state
    }
}

// Function to turn the Brush motor OFF
void BrushOff() {
    if (Brush_Motor_State == 1) { // Only turn off if currently on
        printf("Brush motor OFF\n");
        digitalWrite(Brush_Motor, LOW); // Turn off motor
        Brush_Motor_State = 0; // Update state
    }
}

// Function to turn the Step motor ON
void StepOn() {
    if (Step_Motor_State == 0) { // Only turn on if currently off
        printf("Step motor ON\n");
        digitalWrite(Step_Motor, HIGH); // Turn on motor
        Step_Motor_State = 1; // Update state
    }
}

// Function to turn the Step motor OFF
void StepOff() {
    if (Step_Motor_State == 1) { // Only turn off if currently on
        printf("Step motor OFF\n");
        digitalWrite(Step_Motor, LOW); // Turn off motor
        Step_Motor_State = 0; // Update state
    }
}

// Function to turn the Screw motor ON
void ScrewOn() {
    if (Screw_Motor_State == 0) { // Only turn on if currently off
        printf("Screw motor ON\n");
        digitalWrite(Screw_Motor, HIGH); // Turn on motor
        Screw_Motor_State = 1; // Update state
    }
}

// Function to turn the Screw motor OFF
void ScrewOff() {
    if (Screw_Motor_State == 1) { // Only turn off if currently on
        printf("Screw motor OFF\n");
        digitalWrite(Screw_Motor, LOW); // Turn off motor
        Screw_Motor_State = 0; // Update state
    }
}

// Function to turn the Loader Raise motor ON
void LoadRaiseOn() {
    if (Loader_Raise_State == 0) { // Only turn on if currently off
        printf("Loader Raise ON\n");
        digitalWrite(Loader_Raise, HIGH); // Turn on motor
        Loader_Raise_State = 1; // Update state
    }
}

// Function to turn the Loader Raise motor OFF
void LoadRaiseOff() {
    if (Loader_Raise_State == 1) { // Only turn off if currently on
        printf("Loader Raise OFF\n");
        digitalWrite(Loader_Raise, LOW); // Turn off motor
        Loader_Raise_State = 0; // Update state
    }
}

// Function to turn the Loader Lower motor ON
void LoadLowOn() {
    if (Loader_Lower_State == 0) { // Only turn on if currently off
        printf("Loader Lower ON\n");
        digitalWrite(Loader_Lower, HIGH); // Turn on motor
        Loader_Lower_State = 1; // Update state
    }
}

// Function to turn the Loader Lower motor OFF
void LoadLowOff() {
    if (Loader_Lower_State == 1) { // Only turn off if currently on
        printf("Loader Lower OFF\n");
        digitalWrite(Loader_Lower, LOW); // Turn off motor
        Loader_Lower_State = 0; // Update state
    }
}

// Function to turn off all motors
void TurnOffMotors() {
    BrushOff();
    StepOff();
    ScrewOff();
    LoadRaiseOff();
    LoadLowOff();
    printf("All motors turned OFF\n");
}

int load(void)
{
    if (wiringPiSetupGpio() == -1) { // Use BCM pin numbering
        printf("WiringPi setup failed!\n");
        return 1;
    }

    setup();

    // Example usage
    BrushOn();  // Turn on Brush motor
    sleep(5);   // Keep it on for 5 seconds
    TurnOffMotors(); // Turn off all motors

    ScrewOn();
    sleep(5);
    TurnOffMotors();

    StepOn();   // Turn on Step motor
    sleep(3);   // Keep it on for 3 seconds
    TurnOffMotors(); // Turn off all motors

    return 0;
}