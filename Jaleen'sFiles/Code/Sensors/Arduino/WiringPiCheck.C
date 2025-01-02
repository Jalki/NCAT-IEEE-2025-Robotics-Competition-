#include <wiringPi.h>
#include <stdio.h>

int main(){
    if (wiringPiSetup() == -1){
        printf("wiringPi is not installed or setup failed. \n");
        return 1;
    }
    printf("wiringPi is installed and working!\n");
    return 0;
}