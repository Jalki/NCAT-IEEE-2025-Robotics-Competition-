#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>

#define SPI_CHANNEL 0 // SPI Channel (0 or 1)
#define SPI_SPEED 100000 //SPI speed of 1 MHZ
#define ADC_CHANNEL 0 //ADC channel where KY-018 is connected (0-7)

int readADC(int channel);

int main(void){
    //Init WiringPi
    if (wiringPiSetup() == -1){
        printf("WiringPi setup failed!\n");
        return 1;
    }

    //Initialize SPI
    if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1){
        printf("SPI setup failed!\n");
        return 1;
    }

    while(1){
        int lightValue = readADC(ADC_CHANNEL);
        printf("Light Intensity: %d\n", lightValue);
        //In this block below, add logic based on light value!
        delay(500); //Delay for 500 MS
    }

    return 0;
}

int readADC(int channel){
    if (channel < 0 || channel > 7){
        printf("Invalid ADC channel: %d\n", channel);
        return -1;
    }

    unsigned char buffer[3];
    buffer[0] =1; //start bit
    buffer[1] = (8 + channel) << 4; //Channel configuration
    buffer[2] = 0; //Placeholder for the result

    //Perform the SPI transaction
    wiringPiSPIDataRW(SPI_CHANNEL, buffer, 3);
    int result = ((buffer[1] & 3) << 8) | buffer[2];
    return result;
}
