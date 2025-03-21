#include <wiringPi.h>
#include <stdio.h>

#define MOTOR_A_IN1 23  // GPIO 23
#define MOTOR_A_IN2 24  // GPIO 24
#define MOTOR_B_IN1 08   //GPIO 08
#define MOTOR_B_IN2 07  //GPIO 07
#define MOTOR_C_IN1 22 //

void setup() {

    // Set motor control pins as output
    pinMode(MOTOR_A_IN1, OUTPUT);
    pinMode(MOTOR_A_IN2, OUTPUT);
    pinMode(MOTOR_B_IN1, OUTPUT);
    pinMode(MOTORB_IN2, OUTPUT);

    // Ensure motor is stopped at the start
    digitalWrite(MOTOR_A_IN1, LOW);
    digitalWrite(MOTOR_A_IN2, LOW);
    digitalWrite(MOTOR_B_IN2, LOW);
    digitalWrite(MOTOR_B_IN2, LOW);

    printf("Setup complete. Ready to run motor.\n");
}

void runMotor() {
    printf("Running motor forward...\n");
    digitalWrite(MOTOR_A_IN1, HIGH);
    digitalWrite(MOTOR_A_IN2, LOW);
    delay(2000);  // Motor runs for 2 seconds

    printf("Stopping motor...\n");
    digitalWrite(MOTOR_A_IN1, LOW);
    digitalWrite(MOTOR_A_IN2, LOW);
    delay(2000);  // Pause for 2 seconds

    printf("Running motor in reverse...\n");
    digitalWrite(MOTOR_A_IN1, LOW);
    digitalWrite(MOTOR_A_IN2, HIGH);
    delay(2000);  // Motor runs in reverse for 2 seconds

    printf("Stopping motor again...\n");
    digitalWrite(MOTOR_A_IN1, LOW);
    digitalWrite(MOTOR_A_IN2, LOW);
    delay(2000);
}

int main() {

    
    if (wiringPiSetupGpio() == -1) {
        printf("setup failed");
        return 1;  // Exit if WiringPi fails
    }
    setup();
    while (1) {
        runMotor();
    }
    
    return 0;
}
