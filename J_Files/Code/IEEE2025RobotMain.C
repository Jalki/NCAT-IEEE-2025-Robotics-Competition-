#include <stdio.h>
#include <pthread.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
//#include "PhotoresistorOperation.C"
//#include "IR_Avoidance.C"
//include "AprilTag.C"

//This is the main file for the IEEE 2025 Southeast Con robotics competition. All code is public and open sourced.
//Most of this code is simply a overarching state machine to control what happens in said state, and the switching of states!
int State = 0; //0-Inert State (IS), 1-Calibration State (CS), 2-Signal LED State (SLS), 3-Ambient Navigation State (ANS), 4- Cave Navigation State (CNS), 5- Failed State (FS)

int user; //Integer to look at what user wants (TESTING ONLY!)

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
    switch(State)
    {
        //This is the actual states of the robot!
        case 1: //Calibration State - sensor polling test, motor check
            State = 1;
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

int main(void){
    StateTrans();
    return 0;
}