//#include <Wire.h>
int encoderPulses = 0;
int directionCommand;
int dataIn = 0;
//output/input pins
int loaderLower = 2,
  loaderRaise = 3,
  brush = 7;
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
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#define UART_PORT "/dev/ttyAMA0"
#define BAUDRATE B9600
#include "move.c"
//#include "PhotoresistorOperation.C"
//#include "IR_Avoidance.C"
//include "AprilTag.C"

//This is the main file for the IEEE 2025 Southeast Con robotics competition. All code is public and open sourced.
//Most of this code is simply a overarching state machine to control what happens in said state, and the switching of states!
int State = 0; //0-Inert State (IS), 1-Calibration State (CS), 2-Signal LED State (SLS), 3-Ambient Navigation State (ANS), 4- Cave Navigation State (CNS), 5- Failed State (FS)

int user; //Integer to look at what user wants (TESTING ONLY!)

//Define values


void Inert_State();
void StateTrans();

// Global synchronization variables.
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// Global state variables.
int currentThread = 1;  // 1: Actuators, 2: Sensors, 3: Camera, 4: Data.
bool killswitch = false;

// Helper function: returns the next thread's ID (1?2?3?4?1).
int nextThread(int t) {
    return (t % 4) + 1;
}



// Thread functions
void* actuators_work(void* arg);
void* sensors_work(void* arg);
void* camera_work(void* arg);
void* data_work(void* arg);
void* MThreadRoutine(void* arg);

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
				pthread_t mThread;
				int ret;

				// Run the multithreading test routine via MThreadRoutine.
				ret = pthread_create(&mThread, NULL, MThreadRoutine, NULL);
				if (ret != 0) {
					fprintf(stderr, "Error creating MThreadRoutine thread: %d\n", ret);
					exit(EXIT_FAILURE);
				}
				pthread_join(mThread, NULL);
				printf("All threads finished.\n");
				
            break;
            case 7:
                printf("~Starting Actuators Work Thread (Unit Test)~ \n");
                // Create a new thread that runs the production actuators_work function.
                ret = pthread_create(&testThread, NULL, actuators_work, NULL);
                if (ret != 0) {
                    fprintf(stderr, "Error creating Actuators Work thread: %d\n", ret);
                    exit(EXIT_FAILURE);
                }
                // Let the actuators_work thread run for a short period (e.g., 5 seconds).
                sleep(5);
                // Signal the thread to exit.
                pthread_mutex_lock(&lock);
                killswitch = true;
                pthread_cond_broadcast(&cond);
                pthread_mutex_unlock(&lock);
                // Wait for the thread to finish.
                pthread_join(testThread, NULL);
                printf("Actuators Work Thread finished.\n");
            
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
   
	
	while (1) {//this should eventually be removed
    StateTrans();
	//The FSM should only run once. The FSM should be designed to loop around to run indefinitely.
}
    return 0;
}
void* actuators_work_debug(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        while (currentThread != 1 && !killswitch) {
            printf("Thread 1 (Actuators [debug]) is waiting for its turn...\n");
            pthread_cond_wait(&cond, &lock);
        }
        if (killswitch) {
            pthread_mutex_unlock(&lock);
            break;
        }
        printf("Thread 1 (Actuators [debug]) received message: Multithreading Testing\n");
        sleep(1);  // Simulate work.
        currentThread = nextThread(1);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&lock);
    }
    printf("Thread 1 (Actuators [debug]) exiting.\n");
    return NULL;
}

// Sensors [debug]
void* sensors_work_debug(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        while (currentThread != 2 && !killswitch) {
            printf("Thread 2 (Sensors [debug]) is waiting for its turn...\n");
            pthread_cond_wait(&cond, &lock);
        }
        if (killswitch) {
            pthread_mutex_unlock(&lock);
            break;
        }
        printf("Thread 2 (Sensors [debug]) received message: Multithreading Testing\n");
        sleep(1);
        currentThread = nextThread(2);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&lock);
    }
    printf("Thread 2 (Sensors [debug]) exiting.\n");
    return NULL;
}

// Camera [debug]
void* camera_work_debug(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        while (currentThread != 3 && !killswitch) {
            printf("Thread 3 (Camera [debug]) is waiting for its turn...\n");
            pthread_cond_wait(&cond, &lock);
        }
        if (killswitch) {
            pthread_mutex_unlock(&lock);
            break;
        }
        printf("Thread 3 (Camera [debug]) received message: Multithreading Testing\n");
        sleep(1);
        currentThread = nextThread(3);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&lock);
    }
    printf("Thread 3 (Camera [debug]) exiting.\n");
    return NULL;
}

// Data [debug]
void* data_work_debug(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        while (currentThread != 4 && !killswitch) {
            printf("Thread 4 (Data [debug]) is waiting for its turn...\n");
            pthread_cond_wait(&cond, &lock);
        }
        if (killswitch) {
            pthread_mutex_unlock(&lock);
            break;
        }
        printf("Thread 4 (Data [debug]) received message: Multithreading Testing\n");
        sleep(1);
        currentThread = nextThread(4);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&lock);
    }
    printf("Thread 4 (Data [debug]) exiting.\n");
    return NULL;
}

// =======================
// MThreadRoutine: the multithreading test routine.
// This function creates the four debug threads, lets them run for a while,
// then activates a killswitch to have them exit gracefully.
// =======================
void* MThreadRoutine(void* arg) {
    pthread_t t1, t2, t3, t4;
    int ret;

    ret = pthread_create(&t1, NULL, actuators_work_debug, NULL);
    if (ret != 0) {
        fprintf(stderr, "Error creating Actuators [debug] thread: %d\n", ret);
        exit(EXIT_FAILURE);
    }
    ret = pthread_create(&t2, NULL, sensors_work_debug, NULL);
    if (ret != 0) {
        fprintf(stderr, "Error creating Sensors [debug] thread: %d\n", ret);
        exit(EXIT_FAILURE);
    }
    ret = pthread_create(&t3, NULL, camera_work_debug, NULL);
    if (ret != 0) {
        fprintf(stderr, "Error creating Camera [debug] thread: %d\n", ret);
        exit(EXIT_FAILURE);
    }
    ret = pthread_create(&t4, NULL, data_work_debug, NULL);
    if (ret != 0) {
        fprintf(stderr, "Error creating Data [debug] thread: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    // Let threads run for a while (e.g., 10 seconds).
    sleep(10);

    // Activate killswitch to signal threads to exit.
    pthread_mutex_lock(&lock);
    killswitch = true;
    pthread_cond_broadcast(&cond);  // Wake up any waiting threads.
    pthread_mutex_unlock(&lock);

    // Join threads to clean up.
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);

    return NULL;
}

// =======================
// Empty stub functions for the original thread names.
// These can later be implemented with production code.
// =======================
// Production Actuators Thread
void* actuators_work(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        while (currentThread != 1 && !killswitch) {
            // Actuators thread is waiting for its turn...
            pthread_cond_wait(&cond, &lock);
        }
        if (killswitch) {
            pthread_mutex_unlock(&lock);
            break;
        }
        
        // ----- Production Actuators Work Begin -----
        // TODO: Insert actual actuator control code here.
        // For example: run_motor_control(), update_actuator_state(), etc.
        // Example:
        // run_motor_control();
        // update_actuator_state();
        printf("Hello word");
        // ----- Production Actuators Work End   -----
        
        sleep(1);  // Simulated delay (remove when production code is in place).
        
        currentThread = nextThread(1);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

// Production Sensors Thread
void* sensors_work(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        while (currentThread != 2 && !killswitch) {
            // Sensors thread is waiting for its turn...
            pthread_cond_wait(&cond, &lock);
        }
        if (killswitch) {
            pthread_mutex_unlock(&lock);
            break;
        }
        
        // ----- Production Sensors Work Begin -----
        // TODO: Insert actual sensor reading/processing code here.
        // For example: read_sensors(), process_sensor_data(), etc.
        // Example:
        // read_sensors();
        // process_sensor_data();
        // ----- Production Sensors Work End   -----
        
        sleep(1);  // Simulated delay (remove when production code is in place).
        
        currentThread = nextThread(2);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

// Production Camera Thread
void* camera_work(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        while (currentThread != 3 && !killswitch) {
            // Camera thread is waiting for its turn...
            pthread_cond_wait(&cond, &lock);
        }
        if (killswitch) {
            pthread_mutex_unlock(&lock);
            break;
        }
        
        // ----- Production Camera Work Begin -----
        // TODO: Insert actual camera capture/processing code here.
        // For example: capture_image(), process_image(), etc.
        // Example:
        // capture_image();
        // process_image();
        // ----- Production Camera Work End   -----
        
        sleep(1);  // Simulated delay (remove when production code is in place).
        
        currentThread = nextThread(3);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

// Production Data Thread (e.g., UART communication)
void* data_work(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        while (currentThread != 4 && !killswitch) {
            // Data thread is waiting for its turn...
            pthread_cond_wait(&cond, &lock);
        }
        if (killswitch) {
            pthread_mutex_unlock(&lock);
            break;
        }
        
        // ----- Production Data Work Begin -----
        // TODO: Insert actual data communication code here.
        // For example: read_from_uart(), send_data(), process_received_data(), etc.
        // Example:
        // read_from_uart();
        // process_received_data();
        // ----- Production Data Work End   -----
        
        sleep(1);  // Simulated delay (remove when production code is in place).
        
        currentThread = nextThread(4);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}




/*The threads have been rewritten like this  because:
1. There needs some sort of guaranteed order to ensure data accuracy
[using a boolean in the old manner does not help us in this regard]

2. It did not provide true parallel processing--even though the processes are
running one at a time, this code can be adapted to do concurrent tasks. To
avoid data race conditions, this is intentionally avoided as much as allowed,
but allows easy implement if the need arises.

3. Busy waiting for a boolean allows one thread to run, but 3 others are polling
for a condition--leading to wasted clock cycles.


*/
int setup_uart() {
    int uart_fd = open(UART_PORT, O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart_fd == -1) {
        perror("Failed to open UART");
        return -1;
    }
    
    struct termios options;
    tcgetattr(uart_fd, &options);
    cfsetispeed(&options, BAUDRATE);
    cfsetospeed(&options, BAUDRATE);
    options.c_cflag = CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart_fd, TCIFLUSH);
    tcsetattr(uart_fd, TCSANOW, &options);
    
    return uart_fd;
}

void send_data(int uart_fd, const char *data) {
    if (uart_fd != -1) {
        int count = write(uart_fd, data, strlen(data));
        if (count < 0) {
            perror("UART TX error");
        } else {
            printf("Sent: %s\n", data);
        }
    } else {
        printf("UART port not open.\n");
    }
}

 //gcc -o W top.c  -l wiringPi