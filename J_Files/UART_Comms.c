#include <stdio.h>    // Standard I/O functions
#include <stdlib.h>   // Standard C library
#include <wiringPi.h> // Wiring Pi
#include <fcntl.h>    // File control
#include <unistd.h>   // POSIX API (sleep, read, write)
#include <termios.h>  // Terminal I/O
#include <string.h>   // String functions
#include <errno.h>    // Error handling
#include <sys/time.h> // System time functions
#include <string.h>  // for strlen()
#include "move.c" //This is Arnold Grid code!!!
#include <cmath>
#define UART_PORT "/dev/ttyAMA0" // Change this to your actual serial port

char gyro[256];
char newGyro[256];
char accel[256];
char newaccel[256];

char buffer[256];
int uart_fd = -1;  // Global UART file descriptor
double overrideMag = 5.0;


//Level 1 functions
int movexy(double target_x, double target_y);
int rotate(double angle);
char* uart_read(int fd);
void uart_direction_Write(int fd,
    double x,
    double y,
    double rotation,
    int aux);


// Function to configure UART
void configure_uart(int uart_fd) {
    struct termios options;
    tcgetattr(uart_fd, &options);

    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);

    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    options.c_cflag |= (CLOCAL | CREAD);
    cfmakeraw(&options);

    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 10;

    tcsetattr(uart_fd, TCSANOW, &options);
}

// Function to send a float array as a comma-separated string
void uart_direction_Write(int fd, double x, double y, double rotation, int aux) {
    char data[50];  // Buffer to hold formatted string
    if (aux == 1) {
        snprintf(data, sizeof(data), "%.4f,%.4f,%.4f\n", x, y, rotation);

    } else {
        printf("print alt uart\n");

        snprintf(data, sizeof(data), "%.4f,%.4f,%.4f,%.4f\n", x, y, rotation, rotation);
    }
   
    
    int bytes_written = write(fd, data, strlen(data));
    if (bytes_written < 0) {
        perror("UART Write Error");
    } else {
        printf("Sent: %s", data);
    }
}



//Seperate function to actively write what state the robot is in!
void uart_write_state(int fd, int State)
{
    char data[50];
    snprintf(data, sizeof(data), "%.2d \n", State);

    int bytes_written = write(fd, data, strlen(data));
    if(bytes_written < 0){
        perror("UART Write Error");
    }else {
        printf ("Sent: %s", data);
    }
}


void polluart() {
    char *data = uart_read(uart_fd);
    if (!data) return;

    // strip any trailing '\r' or '\n'
    size_t len = strlen(data);
    while (len > 0 && (data[len-1] == '\r' || data[len-1] == '\n')) {
        data[--len] = '\0';
    }

    // only compare the first 8 bytes to "Complete"
    if (strncmp(data, "complete", 8) == 0) {
        printf("rotation/movement mechanical done\n");
    }
    // only compare the first 5 bytes to "Error"
    else if (strncmp(data, "Error", 5) == 0) {
        printf("Got an error signal.\n");
    }
    else {
        printf("Received something else: %s\n", data);
    }
}


// Function to read data from UART
char* uart_read(int fd) {
    int index = 0;
    char ch;

    while (index < sizeof(buffer) - 1) {
        int n = read(fd, &ch, 1);
        if (n > 0) {
            if (ch == '\n') { // End of data received
                buffer[index] = '\0';
                break;
            }
            buffer[index++] = ch;
        } else if (n < 0) {
            perror("UART Read Error");
            return NULL;  // Return NULL on error
        }
    }
    printf("Received: %s\n", buffer);
    
    return buffer;
}





/*

cd J_Files
gcc -o W UART_Comms.c  -l wiringPi
./W

*/
// Define a structure for a coordinate pair

   /*
typedef struct {
    double x;
    double y;
} Coordinate;


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
    { 6.0, 38.5}  // lower left corner[12]
};
int main() {
    initalizemovement();
    
    // Open UART connection
    uart_fd = open(UART_PORT, O_RDWR | O_NOCTTY);
    if (uart_fd == -1) {
        perror("Unable to open UART");
        return -1;
    }
    configure_uart(uart_fd);


// then down

    //runEdgeCaseTests();
   // rotate(180);
   // rotate(-180);
    while(1){

        sleep(10);
        uart_direction_Write(uart_fd, 3.00, 0.00, 0.00,1);
        sleep(10);
        uart_direction_Write(uart_fd, -3.00, 0.00, 0.00, 0);
    }

    close(uart_fd);
    return 0;
}



*/

//Contributions by Arnold
// New function: movexy()
// Moves the robot to the given target coordinates (in inches relative to the playable area)
// and, if successful, retrieves the delta values and processes them per axis.

double recentdx, recentdy;

int movexy(double target_x, double target_y) {
    // Call moverobotxy() to attempt to move the robot to the target (inches).
    int result = moverobotxy(target_x, target_y);
    double *params = getrobotparams();

    char current_facing = (char) params[2];

    if (result) {
        // Retrieve the movement delta values and primary axis.
        double *mvDeltas = getdeltas();  // mvDeltas[0] = dx, [1] = dy, [2] = primary axis (stored as ASCII)
        double dx = mvDeltas[0];
        double dy = mvDeltas[1];
        recentdx = dx;
        recentdy = dy;
        char primary = (char) mvDeltas[2];
        printf("movexy: Movement succeeded. dx = %.4f in, dy = %.4f in, primary axis = %c\n", 
               dx, dy, primary);
        
        // Check the primary axis and execute code accordingly.
        if (primary == 'x') {
            // Code branch for primary x-axis movement.
            printf("movexy: Primary axis is X. [Insert x-axis processing code here]\n");//PARTICULARLY, UART

            uart_direction_Write(uart_fd, dx, 0.00, 0.00,1);

            polluart();

            printf("sending second set");
            
            uart_direction_Write(uart_fd, 0.00, dy, 0.00,1);
            polluart();

        }
        else if (primary == 'y') {
            // Code branch for primary y-axis movement.
            printf("movexy: Primary axis is Y. [Insert y-axis processing code here]\n");//PARTICULARLY, UART

            uart_direction_Write(uart_fd, 0, dy, 0,1);

            polluart();

            printf("sending second set");
            uart_direction_Write(uart_fd, dx, 0, 0,1);
            polluart();



        }
        else {
            printf("movexy: Unrecognized primary axis '%c'.\n", primary);
        }
    }
    else {
        printf("movexy: Movement to target (%.2f, %.2f) failed.\n", target_x, target_y);
    }
    sleep(3);
    return result;
}




//Parallel function implementation for alignycave
int aligncave() {
    // Call moverobotxy() to attempt to move the robot to the target (inches).
    int result = alignYcave();
    if (result) {
        // Retrieve the movement delta values and primary axis.
        double *mvDeltas = getdeltas();  // mvDeltas[0] = dx, [1] = dy, [2] = primary axis (stored as ASCII)
        double dx = mvDeltas[0];
        double dy = mvDeltas[1];
        char primary = (char) mvDeltas[2];
        printf("movexy: Movement succeeded. dx = %.4f in, dy = %.4f in, primary axis = %c\n", 
               dx, dy, primary);
        
        // Check the primary axis and execute code accordingly.
        if (primary == 'x') {
            // Code branch for primary x-axis movement.
            printf("movexy: Primary axis is X. [Insert x-axis processing code here]\n");//PARTICULARLY, UART

            uart_direction_Write(uart_fd, dx, 0.00, 0.00,1);

            printf("entering while loop for align y");
            polluart();


            printf("sending second set");
            uart_direction_Write(uart_fd, 0.00, dy, 0.00,1);
            polluart();


        }
        else if (primary == 'y') {
            // Code branch for primary y-axis movement.
            printf("movexy: Primary axis is Y. [Insert y-axis processing code here]\n");//PARTICULARLY, UART

            uart_direction_Write(uart_fd, 0, dy, 0,1);
            printf("entering while loop pt2");
            polluart();

            printf("sending second set");
            uart_direction_Write(uart_fd, dx, 0, 0,1);
            polluart();

        }
        else {
            printf("movexy: Unrecognized primary axis '%c'.\n", primary);
        }
    }
    else {
        printf("aligncave: Alignment Y failed.\n");
    }
    return result;
}



// New function: rotate()
// Rotates the robot by the given angle (in degrees) and then retrieves the rotation delta.
// It returns 1 if rotation succeeded, or 0 if it failed.
int rotate(double angle) {

    // Check if rotation is allowed while doing rotation
    if (!rotaterobot((int)angle)) {
        printf("rotate: Rotation failed. Not enough clearance, or incorrect arguments [multiple of 90]\n");
        return 0;
    }
    


    // Retrieve the rotation delta.
    double aDelta = getangledelta();
    printf("rotate: Rotation succeeded. Angle delta: %.2f degrees.\n", aDelta);

    // Process the angle delta if nonzero.
    if (aDelta != 0.0) {
        // [Insert any additional angle delta processing code here.]//PARTICULARLY, UART
        printf("rotate: Processing angle delta: %.2f degrees.\n", aDelta);
        uart_direction_Write(uart_fd, 0, 0, aDelta,1);
        polluart();
        
    }
    sleep(3);
    return 1;
}


int overridexy(double target_x, double target_y, const char *ApplyAxis) {
    // 1) execute the move unconditionally

    int result = movexy(target_x, target_y);//allow to reach target

    if (result) {

        // 2) pull out the recorded deltas
        double *mvDeltas = getdeltas();  // mvDeltas[0] = dx, [1] = dy, [2] = primary axis (stored as ASCII)
       // double dxNorm = mvDeltas[0];
       // double dyNorm= mvDeltas[1];


        double dx = std::copysign(overrideMag, recentdx);
        double dy = std::copysign(overrideMag, recentdy);
        char primary = (char) mvDeltas[2]; //x final motions are more stable than y final motions

        if (strcmp(ApplyAxis, "y") == 0) {
            dx = 0;
        }
        else if (strcmp(ApplyAxis, "x") == 0) {
            dy=0;
        }
        else if (strcmp(ApplyAxis, "xy") == 0) {
            printf("overridexy: Choosing to translate in both axis of general direction\n");
        }

        printf("overridexy: dx = %.4f in, dy = %.4f in, primary axis = %c\n",
            dx, dy, primary);

        
        // 3) dispatch over UART exactly like movexy does
        if (primary == 'x') {
            // first the x?step, the real distance travel
           // uart_direction_Write(uart_fd, dxNorm, 0.0, 0.0,1);// "1" stands for normal movement
           // polluart();//wait vfor this to complete

            // then the y?step

            //uart_direction_Write(uart_fd, 0, dyNorm, 0.0,1);// "1" stands for normal movement
           // polluart();//wait vfor this to complete


            uart_direction_Write(uart_fd, dx, 0.0, 0.0,0);//do overrides after normal translation only
            polluart();
            uart_direction_Write(uart_fd, 0.0, dy, 0.0, 0);
            polluart();

        }
        else if (primary == 'y') {
            // first the y?step
           // uart_direction_Write(uart_fd, 0, dyNorm, 0.0,1);// "1" stands for normal movement
           // polluart();//wait vfor this to complete
            // then the x?step
           // uart_direction_Write(uart_fd, dxNorm, 0.0, 0.0,1);// "1" stands for normal movement
          //  polluart();//wait vfor this to complete


            uart_direction_Write(uart_fd, 0.0, dy, 0.0, 0);//do overrides after normal translation only
            polluart();
            uart_direction_Write(uart_fd, dx, 0.0, 0.0, 0);
            polluart();
        }
        else {
            // fallback if somehow no primary axis was recorded
            uart_direction_Write(uart_fd, dx, dy, 0.0, 0);
            polluart();
        }

    }

    else {printf("overridexy: Robot MUST be in a position to move here to do an override. (because it moves in the general direction of the destination)\n");
    
    }
    // always report ?success? (no safety net here)
    return 1;
}
