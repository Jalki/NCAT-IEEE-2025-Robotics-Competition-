#include <stdio.h>
#include <pthread.h>
//#include <Python.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#define UART_PORT "/dev/ttyAMA0"
#define BAUDRATE B9600
//#include "PhotoresistorOperation.C"
//#include "IR_Avoidance.C"
//#include "AprilTag.C"
#include "UART_Comms.C" //Raspberry Pi Script to send and upload uart data for x, y, and rotation data!
#include "RaspberryPiLoaders.C" //Raspberry Pi Loaders Script to control the loaders!

//This is the main file for the IEEE 2025 Southeast Con robotics competition. All code is public and open sourced.
//Most of this code is simply a overarching state machine to control what happens in said state, and the switching of states!
int State = 0; //0-Inert State (IS), 1-Calibration State (CS), 2-Signal LED State (SLS), 3-Ambient Navigation State (ANS), 4- Cave Navigation State (CNS), 5- Failed State (FS)

int user; //Integer to look at what user wants (TESTING ONLY!)


//Define values
#define NUM_THREADS 4 //Rpi has 4 cores, 1 thread each, meaning 4 threads max
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
int trigger_threads = 0;
int MotorCall = 0; //This calls certain motors from RaspberryPiLoaders. 1-Brush, 2-Step, 3-Screw, 4-LoaderRaise, 5-LoaderLower
void Inert_State();
void StateTrans();

// Thread functions
void* actuators_work(void* arg);
void* sensors_work(void* arg);
void* camera_work(void* arg);
void* data_work(void* arg);


void Calibration_State(){} //The function to manage what would happen in CS

void SignalLED_State(){} //The function to manage what would happen in SLS

void AmbientNavigation_State(){} //The function to manage what would happen in ANS 

void CaveNavigation_State(){} //The function to manage what would happen in CNS

void Failed_State(){} //The function to manage what would happen in FS

//The function to manage the inert state for the rpi, which is for testing certain functionalities and Unit Testing
void Inert_State()
{
    printf("Raspberry Pi is in inert state \n");
    printf("Which testing function do you wish to do?\n");
    printf("     1) Camera, 2) IR Tracking, 3) Photoresistor, 4) Motor Test, 5) Sorting Test, 6) Multithreading Testing, 7) Unit Test\n");
    printf("     Type here: ");
    scanf("%d", &user);
    switch (user)
        {
            case 1:
                printf("~Testing Camera~ \n");
            break;
            case 2:
                printf("~Testing IR Tracking~ \n");
            break;
            case 3:
                printf("~Testing Photoresistor~ \n");
            break;
            case 4:
                printf("~Testing Motor Test~ \n");
            break;
            case 5:
                //setup();
                printf("~Testing Sorting Test~ \n");
            break;
            case 6:
                printf("~Testing Multithreading Testing~ \n");
            break;
            case 7:
                printf("~Testing Unit Test~ \n");
            break;
        default:
            break;
        }
}

void StateTrans() //This function controls the transisting of the state machine
{
    
}

//This function is the thread dedicated to operating actuactors
void* actuactors_work(void* arg)
{
   while (1) {
        if (trigger_threads) {
            pthread_mutex_lock(&print_mutex);
            printf("Thread 1 (Actuators) active! working with what we got!\n");
            while( MotorCall > 0){
                switch(MotorCall)
                {
                    case 1:
                        Brush();
                    case 2:
                        Step();
                    case 3:
                        Screw();
                    case 4:
                        Loader_Raise();
                    case 5:
                        Loader_Lower();
                    default:
                        break;
                }
            }
            pthread_mutex_unlock(&print_mutex);
            break;  // Exit after printing the message
        }
    }
    return NULL;
}

//This function is the thread dedicated to operating sensors
void* sensors_work(void* arg)
{
    while (1) {
        if (trigger_threads) {
            pthread_mutex_lock(&print_mutex);
            printf("Thread 2 (Sensors) received message: Multithreading Testing\n");
            pthread_mutex_unlock(&print_mutex);
            break;  // Exit after printing the message
        }
    }
    return NULL;
}

//This function is the thread dedicated to operating camera work
void* camera_work(void* arg)
{
    while (1) {
        if (trigger_threads) {
            pthread_mutex_lock(&print_mutex);
            printf("Thread 3 (Camera) received message: Multithreading Testing\n");
            pthread_mutex_unlock(&print_mutex);
            break;  // Exit after printing the message
        }
    }
    return NULL;
}

//This function is the thread dedicated to operating data processing and Tx,RX comms
void * data_work(void * arg)
{
     while (1) {
        if (trigger_threads) {
            pthread_mutex_lock(&print_mutex);
            printf("Thread 4 (Data) active! Working on assigned tasks!\n");
            while(1){
                //These two functions should write and then read from uart to give feedback on raspberry pi commands and arduino commands
                uart_direction_Write();
                uart_read();
                switch(State)
                {
                    //This is the actual states of the robot!
                    case 1: //Calibration State - sensor polling test, motor check. This is mostly for our arduino!
                        break;
                    case 2: //Signal LED State - wait LED signal
                        State = 2;
                        break;
                    case 3: //Ambient Light Source State - actively navigating outside cave
                        State = 3;
                        break;
                    case 4: //Cave Navigation State - actively navigating inside cave
                        State = 4;
                        break;
                    case 5: //Failed State - occurs when something goes wrong during the process
                        State = 5;
                        break;
                    default: //Inert State - start robot
                        State = 0;
                        Inert_State();
                        break;
                }
            }
            pthread_mutex_unlock(&print_mutex);
            break;  // Exit after printing the message
        }
    }
    return NULL;
}

int main(void){
    pthread_t thrd_1, thrd_2, thrd_3, thrd_4;

    //Creates threads
    if (pthread_create(&thrd_1, NULL, actuactors_work, NULL) != 0){
        perror("pthread_create for thread 1 failed");
    }

    if (pthread_create(&thrd_2, NULL, sensors_work, NULL) != 0){
        perror("pthread_create for thread 2 failed");
    }

    if (pthread_create(&thrd_3, NULL, camera_work, NULL) != 0){
        perror("pthread_create for thread 3 failed");
    }

    if (pthread_create(&thrd_4, NULL, data_work, NULL) != 0){
        perror("pthread_create for thread 4 failed");
    }
}