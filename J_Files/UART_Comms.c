#include <stdio.h>    // Standard I/O functions
#include <stdlib.h>   // Standard C library
#include <wiringPi.h> // Wiring Pi
#include <fcntl.h>    // File control
#include <unistd.h>   // POSIX API (sleep, read, write)
#include <termios.h>  // Terminal I/O
#include <string.h>   // String functions
#include <errno.h>    // Error handling
#include <sys/time.h> // System time functions

#include "move.c" //This is Arnold Grid code!!!

#define UART_PORT "/dev/ttyAMA0" // Change this to your actual serial port

char gyro[256];
char newGyro[256];
char accel[256];
char newaccel[256];

char buffer[256];

//Level 1 functions
int movexy(double target_x, double target_y);
int rotate(double angle);


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
void uart_direction_Write(int fd, float x, float y, float rotation) {
    char data[50];  // Buffer to hold formatted string
    snprintf(data, sizeof(data), "%.2f,%.2f,%.2f\n", x, y, rotation);
    
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

// Function to read data from UART
void uart_read(int fd) {
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
            return;
        }
    }
    printf("Received: %s\n", buffer);
}

int main() {
    initalizemovement();

    

    while (1) {
        uart_read(uart_fd);
        uart_direction_Write(uart_fd, x, y, rotation);
        sleep(1);
    }

    close(uart_fd);
    return 0;
}



//Contributions by Arnold
// New function: movexy()
// Moves the robot to the given target coordinates (in inches relative to the playable area)
// and, if successful, retrieves the delta values and processes them per axis.
int movexy(double target_x, double target_y) {
    // Call moverobotxy() to attempt to move the robot to the target (inches).
    int result = moverobotxy(target_x, target_y);
    if (result) {
        // Retrieve the movement delta values and primary axis.
        double *mvDeltas = getdeltas();  // mvDeltas[0] = dx, [1] = dy, [2] = primary axis (stored as ASCII)
        double dx = mvDeltas[0];
        double dy = mvDeltas[1];
        char primary = (char) mvDeltas[2];
        printf("movexy: Movement succeeded. dx = %.2f in, dy = %.2f in, primary axis = %c\n", 
               dx, dy, primary);
        
        // Check the primary axis and execute code accordingly.
        if (primary == 'x') {
            // Code branch for primary x-axis movement.
            printf("movexy: Primary axis is X. [Insert x-axis processing code here]\n");//PARTICULARLY, UART
        }
        else if (primary == 'y') {
            // Code branch for primary y-axis movement.
            printf("movexy: Primary axis is Y. [Insert y-axis processing code here]\n");//PARTICULARLY, UART
        }
        else {
            printf("movexy: Unrecognized primary axis '%c'.\n", primary);
        }
    }
    else {
        printf("movexy: Movement to target (%.2f, %.2f) failed.\n", target_x, target_y);
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
    }
    return 1;
}

