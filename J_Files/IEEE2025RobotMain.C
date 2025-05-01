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
#include <stdbool.h>
#include <Python.h>


//#include <Python.h>
//#include </usr/include/python3.11.2/pyconfig-64.h>

#define UART_PORT "/dev/ttyAMA0" // This is Serial 1 for the Arduino!
#define BAUDRATE B9600

#include "UART_Comms.c" // Raspberry Pi Script to send and upload UART data for x, y, and rotation data!
/*Top level functions from this code
movexy(x,y)
aligncave()
rotate(double angle)
*/
#include "RaspberryPiLoaders.C" // Raspberry Pi Loaders Script to control the loaders!

// Hey! Yeah You! If you are reading this and wondering, what the from this code, outside the comments littered here, the IMPORTANTREADTHIS.txt file explains everything!



pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t motor_mutex = PTHREAD_MUTEX_INITIALIZER;
int trigger_threads = 0;
int MotorCall = 0;
int countBall = 0;
int ballexist = 0;
#include <stdio.h>

// Define a structure for a coordinate pair
typedef struct {
    double x;
    double y;
} Coordinate;

// Define the unique states in the sequence
typedef enum {
    WAIT_FOR_LIGHT,
    OUTSIDE_SWEEP,
    UNLOAD_SORT,
    PREP_CAVE,
    CAVE_SWEEP,
    GO_HOME,
    FINISHED,
    idle
} MState;

// Define an array of preset coordinate pairs in the specified order
Coordinate common[] = {//usage:    movexy(common[idx].x, common[idx].y);
    {86.5, 7.5},//below upper stud[0]
    {86.5, 37.0},//above lower stud[1]
    {83.0, 6.0},//left upper stud[2]
    {83.0, 38.5},//left lower stud[3]
    {68.0, 6.0},//upper left corner in cave[4]
    {68.0, 38.5},//lower left corner[5]
    {48.5, 6.0},//upper right corner out cave[6]
    {42.0, 38.0},//left 'G' box[7]
    {48.5, 32.0},//above 'G' box[8]
	{26.5, 3},//G box centre [9]
	{31, 38.5},//home point[10]
    { 6.0,  6.0},  // upper left corner[11]
    { 6.0, 38.5},  // lower left corner[12]
    {17,6},//left upper box [13]
    {23,12.5}, //below upper box [14]
    {36,6}, //right upper box [15]
};

//int State = 1; no longer used
int user;
//int uart_fd = -1;  // Global UART file descriptor does not need to be redefined--uart is being imported with such variables
int running = 1;    // Global flag to control thread execution
MState currentState = idle;


void Inert_State();
void* actuators_work(void* arg);
void* sensors_work(void* arg);
void* camera_work(void* arg);
void* data_work(void* arg);

// Function prototypes for each action
void waitForLight();
void outsideSweep();
void unloadSortBins(const char *prelocation);
void prepCave();
void caveSweep();
void goHome();
int balldetect(const char *filename, int *last_count);
int prepareclearance(char borderdir, char facingfinaldirection);
int point(char targetDir);
void overridequick(char mode, int idx, const char *axis);

//The usleep() function in C suspends execution of the calling thread for the number of microseconds specified in its argument.
//It's part of the unistd.h header and is used for introducing short delays in a program's execution.

// Signal handler to catch CTRL+C and stop threads
void handle_sigint(int sig) {
    printf("\nTerminating program...\n");
    running = 0;
}

// Initial state function for selecting testing functions
void Inert_State() {
    int user;
    printf("Raspberry Pi is in inert state\n");
    printf("Which testing function do you wish to do?\n");
    printf("     1) Camera\n");
    printf("     2) Motor Test\n");
    printf("     3) Sorting Test\n");
    printf("     4) Multithreading Testing\n");
    printf("     5) ROBOT PROCESS\n");
    printf("     Type here: ");
    scanf("%d", &user);

    switch (user) {
        case 1:
            printf("~STARTING Camera~\n");

            break;
        case 2:
            printf("~Testing Motor Test~\n");
            break;
        case 3:
            printf("~Testing Sorting Test~\n");
            break;
        case 4:
            printf("~Testing Multithreading Testing~\n");
            break;
        case 5:
            printf("~Running Routine.t~\n");
            running      = 1;
            currentState = WAIT_FOR_LIGHT;

            break;
        default:
            printf("Invalid selection.\n");
            break;
    }
}


// Thread function for actuator operations
void* actuators_work(void* arg) {


    while (running) {
        if (trigger_threads) {
            pthread_mutex_lock(&print_mutex);
            printf("Thread 1 (Actuators) active! Working with what we got!\n");
            sleep(1);
            pthread_mutex_unlock(&print_mutex);

            while (MotorCall > 0 && running) {
                pthread_mutex_lock(&motor_mutex);
                switch (MotorCall) {
                    case 1:
                        BrushOn();
                        break;  // Turn on Brush motor
                    case 2:
                        StepOn();
                        break;   // Turn on Step motor
                    case 3:
                        ScrewOn();
                        break;  // Turn on Screw motor
                    case 4:
                        LoadRaiseOn();
                        break; // Turn on Loader Raise
                    case 5:
                        LoadLowOn();
                        break;   // Turn on Loader Lower
                    case 0:
                        TurnOffMotors();
                        break; // Turn off all motors
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


//new top level state machine to control the entire competition process

void* data_work(void* arg) {//current setup:
    /*each function is assumed to stay in a loop, and only if it is ready to goto next state,

    */

    // iteration == 0 indicates the first pass;
    // iteration == 1 indicates the second pass through PREP_CAVE and UNLOAD_SORT
    int iteration = 0;


    while (running) {
        switch (currentState) {
            case idle:
                Inert_State();

            case WAIT_FOR_LIGHT:
                waitForLight();//initializes movement grid system
                currentState = OUTSIDE_SWEEP;
                break;

            case OUTSIDE_SWEEP:
               unloadSortBins("o");
                currentState = UNLOAD_SORT;
                break;

            case UNLOAD_SORT:
                unloadSortBins("o");
                // In the first pass, after unloading we move to PREP_CAVE.
                // In the second pass, after unloading we go home.
                if (iteration == 0) {
                    currentState = PREP_CAVE;
                } else { // iteration == 1
                    currentState = GO_HOME;
                }
                break;

            case PREP_CAVE:
                prepCave();
                // In the first pass, after prepping, the next step is cave sweep.
                // In the second pass, after prepping, the next step is unloading.
                if (iteration == 0) {
                    currentState = CAVE_SWEEP;
                } else { // iteration == 1
                    currentState = UNLOAD_SORT;
                }
                break;

            case CAVE_SWEEP:
                //caveSweep();
                // After the cave sweep, we begin the second cycle with PREP_CAVE.
                iteration = 1;
                currentState = PREP_CAVE;
                break;

            case GO_HOME:
                goHome();
                currentState = FINISHED;
                break;

            case FINISHED:
                running = false;
                break;
        }
        // Optional delay between states
        sleep(1);
    }
    printf("State machine completed.\n");

    return NULL;
}

/*
old state machine in data work:

void* data_work(void* arg) {
    while (running) {
        if (trigger_threads) {
            //temporary silenced for testing parallelism
            pthread_mutex_lock(&print_mutex);
            printf("Thread 4 (Data) active! Working on assigned tasks!\n");
            pthread_mutex_unlock(&print_mutex);

            while (running) {
                switch (State) {
                    case 1: //Calibration State, motors are moved to check if they are working properly from the raspberry pi
                        pthread_mutex_lock(&motor_mutex);
                        MotorCall = 1;
                        delay(3000);
                        MotorCall = 2;
                        delay(3000);
                        MotorCall = 3;
                        delay(3000);
                        MotorCall = 4;
                        delay(3000);
                        MotorCall = 5;
                        delay(3000);
                        pthread_mutex_unlock(&motor_mutex);
                        State = 2;
                        break;
                    case 2: //Start Signal State, awaiting for the arduino to be triggered by a photoresistor to tell if its alright for it to start!
                        //int Ard_Start;
                        //do {
                            //Ard_Start = uart_read(uart_fd);
                        //} while (!Ard_Start && running);
                        //State = (Ard_Start == 1) ? 3 : 7;
                       // break;
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


*/
/*
cd J_Files
gcc -o W IEEE2025RobotMain.C  -l wiringPi  $(python3-config --cflags --embed --libs)
./W
*/
//this doesnt really need mutexes but a way to silence the output
void* camera_work(void* arg) {
    // Initialize the Python interpreter.
    while (running) {
    Py_Initialize();

    // Open the Python script file.
    const char *script_path = "/home/SECON2025/Documents/GitHub/NCAT-IEEE-2025-Robotics-Competition-/J_Files/cmm.py";
    PyRun_SimpleString("import sys, os; sys.stdout = open(os.devnull, 'w'); sys.stderr = open(os.devnull, 'w')");//silences output
    FILE *fp = fopen(script_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open %s\n", script_path);
        Py_Finalize();
        return NULL;
    }

    // Run the Python script.
    PyRun_SimpleFile(fp, "cmm.py");
    fclose(fp);

    // Finalize the Python interpreter.
    Py_Finalize();

    return NULL;
    }
}
//This function is the thread dedicated to operating sensors
void* sensors_work(void* arg)
{
    while (running) {
        if (trigger_threads) {
            pthread_mutex_lock(&print_mutex);
            printf("Thread 2 (Sensors) received message: Multithreading Testing\n");
            if (balldetect("ballexist", &countBall))
            {printf("ball detected");
            ballexist = 1;
            }

            else { printf("no ball");
            ballexist = 0;}
            pthread_mutex_unlock(&print_mutex);
            //break;  // Exit after printing the message
            sleep(1);
        }
    }
    return NULL;
}
int balldetect(const char *filename, int *last_count) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        // File not found; create it with a default value of 0.
        f = fopen(filename, "w");
        if (f == NULL) {
            perror("Error creating file");
            return 0;
        }
        fprintf(f, "0");
        fclose(f);
        if (*last_count != 0) {
            *last_count = 0;
            return 1;
        }
        return 0;
    }
   
    int current_value;
    if (fscanf(f, "%d", &current_value) != 1) {
        fclose(f);
        return 0;  // Could not read a valid integer.
    }
    fclose(f);
   
    if (current_value != *last_count) {
        *last_count = current_value;
        return 1;  // The value changed.
    }
    return 0;
}

void measure() {

    movexy(common[10].x - 5, common[10].y);
    sleep(15);
    movexy(common[10].x, common[10].y);
    sleep(20);
    movexy(common[10].x, common[10].y-5);
    sleep(15);
    movexy(common[10].x, common[10].y);
    sleep(20);
}

void quad() {
    movexy(common[10].x + 10, common[10].y);

movexy(common[10].x, common[10].y);


movexy(common[10].x, common[10].y - 10);

movexy(common[10].x, common[10].y);

}

void quad2() {
    overridexy(common[10].x + 10, common[10].y, "x");
    overridexy(common[10].x, common[10].y, "x");

    overridexy(common[10].x, common[10].y - 10, "y");
    overridexy(common[10].x, common[10].y, "y");
}
void waitForLight() {
    printf("State: Wait For Light\n");
    initalizemovement();
    setup();
    loadcmd();

printf("DONE TESTING");
  // }
}
void outsideSweep() {
    
	double *params = getrobotparams();// only current on first call
	
    //TODO: insert brush roller enable here
//LEGEND: * = PASSED, ** MODIFY BASED ON SUGGESTION, # = FAIL
    //robotcmd("sweep");
	movexy(26.5, params[1]);// [this is a crafty way of translating only by one axis, keep this in mind]*
    //move under centre of N box*
	overridexy(26.5,12.5, "y");//validate this position--below box** convert to override
    overridexy(26.5, 38.5, "y");//jostle x-centre wrt n box to lower border*
	movexy(common[10].x, common[10].y);//home*
	prepareclearance('S', 'W'); //automatically point west w/ clearance work*
	
	movexy(common[12].x,common[12].y-1); //clear left side of home->corner
    overridequick('o', 1, "y");//realign any minor drift [this is drift axis optimized]
    movexy(common[13].x, getrobotparams()[1]-1); //move left of N box at current y coordinate[override planned]
    prepareclearance('S', 'N');//point north at border
    overridexy(common[13].x, 38.5, "y");//ensure robot is flush lower wall
	movexy(common[13].x, common[13].y);//clear left of N box upwards
    //we have cleared left of n box, below it, and left of home.

    overridexy(common[13].x, 38.5, "y");//ensure robot is again flush with lower wall
	movexy(common[10].x, common[10].y);//go home [should go y-x]
	prepareclearance('S', 'E');//point towards G box
	//direction: E
	overridexy(common[7].x, common[7].y, "x");//left of G box [pass],jostle to it also
	movexy(common[10].x, common[10].y);//home
	movexy(getrobotparams()[0], common[8].y);//going up above G box.
    //home x centrered,  on y-height above G box, we will clear this row summarily
	overridexy(common[8].x, common[8].y, "x");//above G box, jostle to r wall.

	//assumption: robot is above G box
	movexy(common[8].x - 3, getrobotparams()[1]);//move away from R wall, maintain y
	prepareclearance('S', 'N');//prepare for pseudo sweep pointing north
	overridexy(common[15].x, getrobotparams()[1], "x"); //GENIUS way of once again taking advantage of predetermined coords
	//this is right stide, x-aligned wrt N box, and y-aligned above G box
    printf("START OF INITIAL SWEEP");
	double oldy = getrobotparams()[1];
	double maxpos = 48.5;

	while (getrobotparams()[0] <= maxpos && running)//start YOLO oabject detection concurrency test
	{
		
		if (ballexist){//complete sweep: use ! conditional [w/o camera or parts]
			overridexy(getrobotparams()[0], 6, "y");//automatically jostle for drifting
			sleep(3);
			movexy(getrobotparams()[0], oldy);//do not jostle, this will misalign
		}
        if ((maxpos - getrobotparams()[0]) <= 6 && (maxpos != getrobotparams()[0])){//horizontal adaptative movement towards right wall
            double currposx = getrobotparams()[0];
            overridexy(currposx + (maxpos - currposx), oldy, "x");//override only b.c its expected to ram into a wall
        } else if ((maxpos - getrobotparams()[0]) > 6){
           // printf("maxpos = %.2f, current X position = %.2f\n", maxpos, params[0]);
            movexy(getrobotparams()[0]+6, oldy);
        } else {
            break;
        }

		sleep(.25);
	}
	
	aligncave();
    point('E'); //end of sweep
	printf("END OF INITIAL SWEEP");
    sleep(200);
	
}
/*Coordinate common[] = {//usage:    movexy(common[idx].x, common[idx].y);
    {86.5, 7.5},//below upper stud[0]
    {86.5, 37.0},//above lower stud[1]
    {83.0, 6.0},//left upper stud[2]
    {83.0, 38.5},//left lower stud[3]
    {68.0, 6.0},//upper left corner in cave[4]
    {68.0, 38.5},//lower left corner[5]
    {48.5, 6.0},//upper right corner out cave[6]
    {42.0, 38.0},//left 'G' box[7]
    {48.5, 32.0},//above 'G' box[8]
	{26.5, 3},//G box centre [9]
	{31, 38.5},//home point[10]
    { 6.0,  6.0},  // upper left corner[11]
    { 6.0, 38.5},  // lower left corner[12]
    {17,6},//left upper box [13]
    {20,18}, //below upper box [14]
    {29,18}, //right upper box [15]
};

*/
void unloadSortBins(const char *prelocation) {
    setup();
    loadcmd();

    //keep in mind to try to keep this simple as we are doing this twice
    printf("State: Unload and Sort to Bins\n");
    // Insert code for unloading and sorting into bins here.
    if (strcmp(prelocation, "i") == 0) {
        overridequick('i', 1, "x");//jostle lower left corner of cave
        movexy(common[5].x+3, common[5].y);//move over 3 units
        prepareclearance('S', 'N');//point n if not already
    }
    else if(strcmp(prelocation, "o")==0) {
        //nothing
        
    }

    aligncave();
    movexy(common[12].x-1, common[12].y);//get drift correction to a common corner

    overridexy(common[12].x, common[12].y-1, "x");
    overridexy(common[12].x, common[12].y, "y");

    aligncave();
    prepareclearance('W', 'S');
    overridexy(common[11].x+1,common[11].y,"y");
    overridexy(common[11].x,common[11].y,"x");

    overridexy(common[14].x, common[14].y, "x");//move next to N box to unload
    robotcmd("unloadN");
    sleep(5);
    robotcmd("off");
    //TODO: insert, the loader up, loader down, off brush rolker,
    //start sorting mechanism for G container
    overridexy(common[12].x+6, common[12].y, "y"); //jostle towards LC
    prepareclearance('S', 'W');//point north to unload
    overridexy(common[7].x, common[7].y, "x");
    overridexy(common[8].x, common[8].y-1, "x");
    overridexy(common[8].x, common[8].y, "y");
    robotcmd("unloadG");
    sleep(5);
    robotcmd("off");
    //todo: ONLY do G box sorting. everything is unloaded already.


    sleep(2);
}

void prepCave() {
    alignYcave();//any x, y coordinate should be accessible

}

/*Coordinate common[] = {//usage:    movexy(common[idx].x, common[idx].y);
    {86.5, 7.5},//below upper stud[0]
    {86.5, 37.0},//above lower stud[1]
    {83.0, 6.0},//left upper stud[2]
    {83.0, 38.5},//left lower stud[3]
    {68.0, 6.0},//upper left corner in cave[4]
    {68.0, 38.5},//lower left corner[5]
    {48.5, 6.0},//upper right corner out cave[6]
    {42.0, 38.0},//left 'G' box[7]
    {48.5, 32.0},//above 'G' box[8]
	{26.5, 3},//G box centre [9]
	{31, 38.5}//home point[10]
};
*/

void caveSweep() {
    printf("State: Cave Sweep\n");
   /*
   plan: plumb all the way right. retract to original position.
   sweep up and turn to center, then go down, return to center, up to x-oriented position left of studs. then move towards x orientation
   right in between studs [test left, then upwards to catch stray parts]. repeat to bottom side.
   
   as a result, this is depicted in a while loop
   */
    
    aligncave();
    double oldy = getrobotparams()[1];//basically cave aligned y value

    moverobotxy(common[4].x, oldy);//cave y aligned, move towards xaligned #4 zone


    double oldx = getrobotparams()[0];//x aligned wrt zone #4
    double Xmaxpos = 86.5;

    prepareclearance('W', 'E');//point right w left side precaution
    overridexy(Xmaxpos, oldy, "x");//shovel towards right wall, jostling it
    moverobotxy(oldx, oldy);//return to old position below zone 4, cave aligned
    point('N');//restore original orientation


    while (getrobotparams()[0] <= Xmaxpos && running)//start YOLO oabject detection concurrency test4
    //this constitutes the entiriety of the sweep
    {
        
        if (ballexist){//y axis movement upward only
            overridexy(getrobotparams()[0], 6, "y");//jostle against upper walls
            sleep(3);
            movexy(getrobotparams()[0], oldy);//do not jostle
        }

        //at this point, the robot is in the y-centerline [or should be]
        point('S');
        //begin southbound sweep

        if (ballexist){//y axis movement downward only
            overridexy(getrobotparams()[0], 38.5, "y");
            sleep(3);
            moverobotxy(getrobotparams()[0], oldy);
        }

        point('N');//return to point up


        //adaptive horizontal sweep to certain extent
        if ((Xmaxpos - getrobotparams()[0]) <= 6 && (Xmaxpos != getrobotparams()[0])){
            double currposx = getrobotparams()[0];
            moverobotxy(currposx + (Xmaxpos - currposx), oldy);
        } else if ((Xmaxpos - getrobotparams()[0]) > 6){
            // printf("Xmaxpos = %.2f, current X position = %.2f\n", Xmaxpos, params[0]);
            moverobotxy(getrobotparams()[0]+6, oldy);
        } else {
            break;
        }

        sleep(.25);
    }

    //at this point, we are very close to the end of the right side. there is only one set of movements to do
    overridexy(Xmaxpos, oldy, "x");//jostle R wall

    if (ballexist){//y axis movement upward only
        moverobotxy(getrobotparams()[0], common[0].y);//todo: robot WILL drift left
        sleep(3);
        moverobotxy(getrobotparams()[0], oldy);
    }

    //at this point, the robot is in the y-centerline [or should be]
    //it should be noted that proper clearance is required--use the appropriate function instead
    prepareclearance('E', 'S');
    overridexy(Xmaxpos, oldy, "x");//jostle R wall
    
    //begin southbound sweep

    if (ballexist){//y axis movement downward only
        moverobotxy(getrobotparams()[0], common[1].y);
        sleep(3);
        moverobotxy(getrobotparams()[0], oldy);
    }

    alignYcave(); //verify y alignment before next state


    
}

void goHome() {
    printf("State: Go Home\n");
    // Insert code for returning home here.
    sleep(2);
}

int point(char targetDir) {
    // Get current robot parameters (the 3rd element is the current facing as an ASCII code)
    double *params = getrobotparams();
    char currentDir = (char) params[2];

    int currentIndex, targetIndex;
    // Map current direction to an index: N=0, E=1, S=2, W=3.
    switch (currentDir) {
        case 'N': currentIndex = 0; break;
        case 'E': currentIndex = 1; break;
        case 'S': currentIndex = 2; break;
        case 'W': currentIndex = 3; break;
        default:
            printf("point: Unknown current facing '%c'.\n", currentDir);
            return 0;
    }
    // Map the target direction to an index.
    switch (targetDir) {
        case 'N': targetIndex = 0; break;
        case 'E': targetIndex = 1; break;
        case 'S': targetIndex = 2; break;
        case 'W': targetIndex = 3; break;
        default:
            printf("point: Invalid target direction '%c'.\n", targetDir);
            return 0;
    }

    // Compute the angle difference.
    // Using the formula: angleDiff = -(targetIndex - currentIndex) * 90.
    int angleDiff = -(targetIndex - currentIndex) * 90;
    printf("point: Current facing '%c' (index %d), target facing '%c' (index %d), angleDiff = %d\n",
           currentDir, currentIndex, targetDir, targetIndex, angleDiff);

    // Call the rotation function.
    if (rotate(angleDiff)) {
        printf("point: Successfully pointed to '%c'.\n", targetDir);
        return 1;
    } else {
        printf("point: Failed to rotate to '%c'.\n", targetDir);
        return 0;
    }
}

int prepareclearance(char borderdir, char facingfinaldirection) {
    // Retrieve current parameters from the global getrobotparams() function.
    // getrobotparams() returns a static array: [pos_x, pos_y, (double)robot_dir]
    double *params = getrobotparams();
    double current_x = params[0];  // in inches relative to playable area
    double current_y = params[1];
    char current_facing = (char) params[2];
    printf("prepareclearance: Current parameters: (%.2f, %.2f) facing %c\n", 
           current_x, current_y, current_facing);
    
    // Save original position (for undoing the clearance move)
    double orig_x = current_x;
    double orig_y = current_y;
    
    // Calculate clearance target position based on the wall (border) direction.
    // For a wall on the south, move up 3 inches (decrease y);
    // for a wall on the north, move down 3 inches (increase y);
    // for a wall on the west, move right 3 inches (increase x);
    // for a wall on the east, move left 3 inches (decrease x).
    double clearance_x = current_x;
    double clearance_y = current_y;
    if (borderdir == 'S' || borderdir == 's') {
        clearance_y = current_y - 3.0;
        printf("prepareclearance: Detected south border. Moving up 3 inches for clearance.\n");
    } else if (borderdir == 'N' || borderdir == 'n') {
        clearance_y = current_y + 3.0;
        printf("prepareclearance: Detected north border. Moving down 3 inches for clearance.\n");
    } else if (borderdir == 'W' || borderdir == 'w') {
        clearance_x = current_x + 3.0;
        printf("prepareclearance: Detected west border. Moving right 3 inches for clearance.\n");
    } else if (borderdir == 'E' || borderdir == 'e') {
        clearance_x = current_x - 3.0;
        printf("prepareclearance: Detected east border. Moving left 3 inches for clearance.\n");
    } else {
        printf("prepareclearance: Unknown border direction '%c'.\n", borderdir);
        return 0;
    }
    
    // Perform the clearance move by calling moverobotxy with the new target.
    if (!movexy(clearance_x, clearance_y)) {
        printf("prepareclearance: Clearance move failed.\n");
        return 0;
    }
    
    // Retrieve the new parameters after clearance move.
    params = getrobotparams();
    current_facing = (char) params[2];
    printf("prepareclearance: After clearance move, facing is now %c\n", current_facing);
    
    // Compute the rotation required to achieve the final desired facing.
    // Map facing characters to indices: N=0, E=1, S=2, W=3.
    int currentIndex, targetIndex;
    switch (current_facing) {
        case 'N': currentIndex = 0; break;
        case 'E': currentIndex = 1; break;
        case 'S': currentIndex = 2; break;
        case 'W': currentIndex = 3; break;
        default:  currentIndex = 0; break;
    }
    switch (facingfinaldirection) {
        case 'N': targetIndex = 0; break;
        case 'E': targetIndex = 1; break;
        case 'S': targetIndex = 2; break;
        case 'W': targetIndex = 3; break;
        default: targetIndex = 0; break;
    }
    int angleDiff = -(targetIndex - currentIndex) * 90;
    while (angleDiff > 180)  angleDiff -= 360;
    while (angleDiff < -180) angleDiff += 360;
    printf("prepareclearance: Rotating robot by %d to face %c\n", angleDiff, facingfinaldirection);
    
    // rotaterobot the robot by the computed angle using the new rotaterobot() function.
    if (!rotate((double)angleDiff)) {
        printf("prepareclearance: Rotation failed.\n");
        return 0;
    }
    
    // Finally, undo the clearance move by returning to the original position.
    printf("prepareclearance: Undoing clearance move: Returning to original position (%.2f, %.2f)\n", orig_x, orig_y);
    if (!movexy(orig_x, orig_y)) {
        printf("prepareclearance: Undo clearance move failed.\n");
        return 0;
    }
    
    // Print final robot parameters.
    params = getrobotparams();
    printf("prepareclearance: Final robot parameters: (%.2f, %.2f) facing %c\n", 
           params[0], params[1], (char)params[2]);
    
    // (Optionally, you can now record the deltas or send them via UART.)
    return 1;
}

// Main function to initialize UART, create threads, and manage execution
int main(void) {
    signal(SIGINT, handle_sigint); // Catch SIGINT (CTRL+C) to exit cleanly
    // Initialize the Python interpreter
    if (wiringPiSetupGpio() == -1) { // Use BCM pin numbering
        printf("WiringPi setup failed!\n");
        return 1;
    }
    //Py_Initialize();


    printf("opening uart");
    // Open UART connection
    uart_fd = open(UART_PORT, O_RDWR | O_NOCTTY);
    if (uart_fd == -1) {
        perror("Unable to open UART");
        return -1;
    }
    configure_uart(uart_fd);

    printf("uart configured.");
    
    // Create threads
    pthread_t thrd_1, thrd_2, thrd_3, thrd_4;
    trigger_threads = 1;  // Ensure threads are triggered to run


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
    //Py_Finalize();

    return 0;
}

void overridequick(char mode, int idx, const char *axis) {
    int mapIdx = -1;
    if (mode == 'o') {
        switch (idx) {
            case 1: mapIdx = 12; break;
            case 2: mapIdx = 11; break;
            case 3: mapIdx =  6; break;
            case 4: mapIdx =  7; break;
        }
    }
    else if (mode == 'i') {
        switch (idx) {
            case 1: mapIdx =  5; break;
            case 2: mapIdx =  4; break;
            case 3: mapIdx =  0; break;
            case 4: mapIdx =  1; break;
        }
    }
    if (mapIdx >= 0) {
        overridexy(common[mapIdx].x, common[mapIdx].y, axis);//auto optimized movement for drift
    }
}