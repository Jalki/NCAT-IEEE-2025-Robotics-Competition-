#include <wiringPi.h>
#include <stdio.h>

#define SENSOR_PIN 8 // GPIO pin 8
#define ENABLE_PIN 7 // GPIO pin 7

int IR_AVOID(void){
    //Setup WiringPi
    if(wiringPiSetup() == -1){
        printf("WiringPi setup failed\n");
        return 1;
    }

    pinMode(SENSOR_PIN, INPUT);
    pinMode(ENABLE_PIN, OUTPUT);

    while(1){
        //Set enable pin to high
        digitalWrite(ENABLE_PIN, HIGH);
        //Read the sensor pin value
        int val = digitalRead(SENSOR_PIN);

        if(val == LOW){
            printf("BAD too close \n");
        }else{
            printf("GOOD not close!\n");
        }
        //print sensor value
        printf("Sensor Value: %d\n", val);

        delay(500);
    }

    return 0;
}