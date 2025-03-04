#include <stdio.h>
#include <wiringPi.h>
#include <sys/time.h>

#define TRIG1 18  // GPIO 18
#define ECHO1 24  // GPIO 24
#define TRIG2 23  // GPIO 23
#define ECHO2 25  // GPIO 25
//#define TRIG3 27  // GPIO 27
//#define ECHO3 22  // GPIO 22

// Function to get the current time in microseconds
long getMicrotime() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    return currentTime.tv_sec * 1000000 + currentTime.tv_usec;
}

// Function to get distance from a specific sensor
double getDistance(int trig, int echo) {
    long startTime, stopTime;
    double distance;

    // Send 10us pulse to trigger pin
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);

    // Wait for echo pin to go HIGH
    while (digitalRead(echo) == LOW);
    startTime = getMicrotime();

    // Wait for echo pin to go LOW
    while (digitalRead(echo) == HIGH);
    stopTime = getMicrotime();

    // Calculate distance in cm (speed of sound is ~34300 cm/s)
    distance = (double)(stopTime - startTime) * 0.0343 / 2.0;

    return distance;
}

int main() {
    if (wiringPiSetupGpio() == -1) {
        printf("WiringPi initialization failed!\n");
        return 1;
    }

    pinMode(TRIG1, OUTPUT);
    pinMode(ECHO1, INPUT);
    pinMode(TRIG2, OUTPUT);
    pinMode(ECHO2, INPUT);
    //pinMode(TRIG3, OUTPUT);
    //pinMode(ECHO3, INPUT);

    printf("Ultrasonic Sensors Initialized. Measuring Distance...\n");

    while (1) {
        double distance1 = getDistance(TRIG1, ECHO1);
        double distance2 = getDistance(TRIG2, ECHO2);
        //double distance3 = getDistance(TRIG3, ECHO3);
        printf("Sensor 1 Distance: %.2f cm\n", distance1);
        printf("Sensor 2 Distance: %.2f cm\n", distance2);
        //printf("Sensor 3 Distance: %.2f cm\n", distance3);
        delay(500);  // Wait 500ms before next reading
    }

    return 0;
}
