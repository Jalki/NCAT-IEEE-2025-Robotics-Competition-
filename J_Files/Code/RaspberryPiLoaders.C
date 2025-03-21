#include <wiringPi.h>
#include <softPwm.h>

#define AIN1  4  // GPIO23
#define AIN2  5  // GPIO24
#define PWMA  6  // GPIO25
#define BIN1 12  // GPIO10
#define BIN2 13  // GPIO9
#define PWMB 14  // GPIO11

void setup() {
    wiringPiSetupGpio(); // Initialize wiringPi using BCM GPIO numbering
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
    softPwmCreate(PWMA, 0, 100); // Initialize software PWM on PWMA
    softPwmCreate(PWMB, 0, 100); // Initialize software PWM on PWMB
}

void setMotorA(int speed, int direction) {
    digitalWrite(AIN1, direction);
    digitalWrite(AIN2, !direction);
    softPwmWrite(PWMA, speed);
}

void setMotorB(int speed, int direction) {
    digitalWrite(BIN1, direction);
    digitalWrite(BIN2, !direction);
    softPwmWrite(PWMB, speed);
}

int main() {
    setup();

    // Example: Run Motor A forward at 75% speed
    setMotorA(75, 1);

    // Example: Run Motor B backward at 50% speed
    setMotorB(50, 0);

    // Run motors for 5 seconds
    delay(5000);

    // Stop motors
    setMotorA(0, 0);
    setMotorB(0, 0);

    return 0;
}
