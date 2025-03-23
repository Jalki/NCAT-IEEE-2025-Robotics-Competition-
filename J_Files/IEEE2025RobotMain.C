#include <stdio.h>
#include <pthread.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <Python.h>

#define UART_PORT "/dev/ttyAMA0" // This is Serial 1 for the Arduino!
#define BAUDRATE B9600

#include "UART_Comms.C" // Raspberry Pi Script to send and upload UART data for x, y, and rotation data!
#include "RaspberryPiLoaders.C" // Raspberry Pi Loaders Script to control the loaders!

int State = 0;
int user;
int uart_fd = -1;  // Global UART file descriptor
int running = 1;    // Global flag to control thread execution

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t motor_mutex = PTHREAD_MUTEX_INITIALIZER;
int trigger_threads = 0;
int MotorCall = 0;

void Inert_State();
void* actuators_work(void* arg);
void* sensors_work(void* arg);
void* camera_work(void* arg);
void* data_work(void* arg);

//The usleep() function in C suspends execution of the calling thread for the number of microseconds specified in its argument. 
//It's part of the unistd.h header and is used for introducing short delays in a program's execution.

// Signal handler to catch CTRL+C and stop threads 
void handle_sigint(int sig) {
    printf("\nTerminating program...\n");
    running = 0;
}

// Initial state function for selecting testing functions
void Inert_State() {
    printf("Raspberry Pi is in inert state \n");
    printf("Which testing function do you wish to do?\n");
    printf("     1) Camera, 2) IR Tracking, 3) Photoresistor, 4) Motor Test, 5) Sorting Test, 6) Multithreading Testing, 7) Unit Test\n");
    printf("     Type here: ");
    scanf("%d", &user);
    switch (user) {
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

// Thread function for actuator operations
void* actuators_work(void* arg) {
    while (running) {
        if (trigger_threads) {
            pthread_mutex_lock(&print_mutex);
            printf("Thread 1 (Actuators) active! Working with what we got!\n");
            pthread_mutex_unlock(&print_mutex);
            while (MotorCall > 0 && running) {
                pthread_mutex_lock(&motor_mutex);
                switch (MotorCall) {
                    case 1: 
                        Brush(); 
                        break;
                    case 2: 
                        Step(); 
                        break;
                    case 3: 
                        Screw(); 
                        break;
                    case 4: 
                        Loader_Raise(); 
                        break;
                    case 5: 
                        Loader_Lower(); 
                        break;
                    default: 
                        break;
                }
                pthread_mutex_unlock(&motor_mutex);
                usleep(50000);
            }
        }
    }
    return NULL;
}

// Thread function for handling data processing and motor control based on state machine
void* data_work(void* arg) {
    while (running) {
        if (trigger_threads) {
            pthread_mutex_lock(&print_mutex);
            printf("Thread 4 (Data) active! Working on assigned tasks!\n");
            pthread_mutex_unlock(&print_mutex);

            while (running) {
                switch (State) {
                    case 1: //Calibration State, motors are moved to check if they are working properly from the raspberry pi
                        pthread_mutex_lock(&motor_mutex);
                        MotorCall = 1; 
                        usleep(300000);
                        MotorCall = 2; 
                        usleep(300000);
                        MotorCall = 3; 
                        usleep(300000);
                        MotorCall = 4; 
                        usleep(300000);
                        MotorCall = 5; 
                        usleep(400000);
                        pthread_mutex_unlock(&motor_mutex);
                        State = 2;
                        break;
                    case 2: //Start Signal State, awaiting for the arduino to be triggered by a photoresistor to tell if its alright for it to start!
                        int Ard_Start;
                        do {
                            Ard_Start = uart_read(uart_fd);
                        } while (!Ard_Start && running);
                        State = (Ard_Start == 1) ? 3 : 7;
                        break;
                    case 3: //Outside of Cave State, the brush motor should always be active!
                        pthread_mutex_lock(&motor_mutex);
                        MotorCall = 1;
                        pthread_mutex_unlock(&motor_mutex);
                        break;
                    case 4: //Inside of Cave State, the brush motor should always be active!
                        pthread_mutex_lock(&motor_mutex);
                        MotorCall = 1;
                        pthread_mutex_unlock(&motor_mutex);
                        break;
                    case 5: //Loader Operation State, should raise then lower the motor
                        pthread_mutex_lock(&motor_mutex);
                        MotorCall = 4; 
                        usleep(1500000);
                        MotorCall = 5; 
                        usleep(1500000);
                        MotorCall = 1;
                        pthread_mutex_unlock(&motor_mutex);
                        break;
                    case 6: //Sorting Operation State, should work the step and screw sorting operation state
                        pthread_mutex_lock(&motor_mutex);
                        MotorCall = 2; 
                        usleep(4000000);
                        MotorCall = 3; 
                        usleep(4000000);
                        MotorCall = 0;
                        pthread_mutex_unlock(&motor_mutex);
                        break;
                    case 7:
                        State = 5;
                        break;
                    default:
                        State = 0;
                        Inert_State();
                        break;
                }
                usleep(50000);
            }
        }
    }
    return NULL;
}

void* camera_work(void* arg) {
    while (running) {
        if (trigger_threads) {
            pthread_mutex_lock(&print_mutex);
            printf("Thread 3 (Camera) active! Detecting AprilTags...\n");
            pthread_mutex_unlock(&print_mutex);

            // Import your Python module
            PyObject* pModule = PyImport_ImportModule("apriltag_detection");
            if (pModule == NULL) {
                PyErr_Print();
                continue;
            }

            // Call a function from your Python module
            PyObject* pFunc = PyObject_GetAttrString(pModule, "detect_apriltags");
            if (pFunc && PyCallable_Check(pFunc)) {
                PyObject* pResult = PyObject_CallObject(pFunc, NULL);
                if (pResult != NULL) {
                    // Process the result (e.g., parse AprilTag data)
                    // You can use PyArg_ParseTuple to extract specific data
                    Py_DECREF(pResult);
                } else {
                    PyErr_Print();
                }
            } else {
                PyErr_Print();
            }

            // Clean up
            Py_XDECREF(pFunc);
            Py_DECREF(pModule);
        }
        usleep(100000);  // Sleep for 100ms to avoid busy-waiting
    }
    return NULL;
}


// Main function to initialize UART, create threads, and manage execution
int main(void) {
    signal(SIGINT, handle_sigint); // Catch SIGINT (CTRL+C) to exit cleanly
    // Initialize the Python interpreter
    Py_Initialize();

    // Open UART connection
    uart_fd = open(UART_PORT, O_RDWR | O_NOCTTY);
    if (uart_fd == -1) {
        perror("Unable to open UART");
        return -1;
    }

    // Configure UART
    if (configure_uart(uart_fd) < 0) {
        perror("Failed to configure UART");
        close(uart_fd);
        return -1;
    }

    // Create threads
    pthread_t thrd_1, thrd_2, thrd_3, thrd_4;

    if (pthread_create(&thrd_1, NULL, actuators_work, NULL) != 0) {
        perror("pthread_create for thread 1 failed");
    }
    if (pthread_create(&thrd_2, NULL, sensors_work, NULL) != 0) {
        perror("pthread_create for thread 2 failed");
    }
    if (pthread_create(&thrd_3, NULL, camera_work, NULL) != 0) {
        perror("pthread_create for thread 3 failed");
    }
    if (pthread_create(&thrd_4, NULL, data_work, NULL) != 0) {
        perror("pthread_create for thread 4 failed");
    }

    // Join threads for a clean exit
    pthread_join(thrd_1, NULL);
    pthread_join(thrd_2, NULL);
    pthread_join(thrd_3, NULL);
    pthread_join(thrd_4, NULL);

    // Close UART before exiting
    close(uart_fd);
    printf("Program exited cleanly.\n");
    // Finalize the Python interpreter before exiting
    Py_Finalize();

    return 0;
}
