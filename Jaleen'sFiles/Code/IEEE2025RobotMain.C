#include <stdio.h>
#include <pthread.h>
//#include "PhotoresistorOperation.C"
//#include "IR_Avoidance.C"
#include "AprilTag.C"

//This is the main file for the IEEE 2025 Southeast Con robotics competition. All code is public and open sourced.
//Most of this code is simply a overarching state machine to control what happens in said state, and the switching of states!
int State = 0; //0-Inert State (IS), 1-Calibration State (CS), 2-Signal LED State (SLS), 3-Ambient Navigation State (ANS), 4- Cave Navigation State (CNS), 5- Failed State (FS)

int user; //Integer to look at what user wants (TESTING ONLY!)

void Calibration_State(){} //The function to manage what would happen in CS

void SignalLED_State(){} //The function to manage what would happen in SLS

void AmbientNavigation_State(){} //The function to manage what would happen in ANS 

void CaveNavigation_State(){} //The function to manage what would happen in CNS

void Failed_State(){} //The function to manage what would happen in FS

void StateTrans() //This function controls the transisting of the state machine
{
    switch(State)
    {
        case 1:
            State = 1;
            break;
        case 2:
            State = 2;
            break;
        case 3:
            State = 3;
            break;
        case 4:
            State = 4;
            break;
        case 5:
            State = 5;
            break;
        default:
            State = 0;
            printf("Raspberry Pi is in inert state");
            printf("Which testing function do you wish to do? 1)April Tag Detection Software, 2) IR Tracking, 3)Photoresistor, 4) Camera, 5)Actuactors, 6) Multithreading testing");
            scanf("%d", &user);
            switch (user)
            {
            case 1:
                ;
                break;
            
            default:
                break;
            }
            break;
    }
}

int main(void){
    StateTrans();
    return 0;
}