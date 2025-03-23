#include <stdio.h>    // Standard I/O functions
#include <stdlib.h>   // Standard C library
#include <wiringPi.h> // Wiring Pi
#include <fcntl.h>    // File control
#include <unistd.h>   // POSIX API (sleep, read, write)
#include <termios.h>  // Terminal I/O
#include <string.h>   // String functions
#include <errno.h>    // Error handling
#include <sys/time.h> // System time functions
#include <softPwm.h>

// Define BCM pins for the motors
#define Brush_Motor 23 // BCM Pin 23
#define Step_Motor 24 // BCM Pin 24
#define Screw_Motor 25 // BCM Pin 25
#define Loader_Raise 8 // BCM Pin 08
#define Loader_Lower 7 // BCM Pin 07

#define UART_PORT "/dev/ttyAMA0" // Change this to your actual serial port

char buffer[256];

void setup()
{
    // Set motor control pins to output
    pinMode(Brush_Motor, OUTPUT);
    pinMode(Step_Motor, OUTPUT);
    pinMode(Screw_Motor, OUTPUT);
    pinMode(Loader_Lower, OUTPUT);
    pinMode(Loader_Raise, OUTPUT);
    digitalWrite(Loader_Lower, LOW);
    digitalWrite(Loader_Raise, LOW);
}

void Brush(){
    printf("Brush moving\n");
    digitalWrite(Brush_Motor, HIGH); // Turn on motor
    delay(5000); // Run motor for 500ms
    digitalWrite(Brush_Motor, LOW); // Turn off motor
}

void Step(){
    printf("Step moving\n");
    digitalWrite(Step_Motor, HIGH); // Turn on motor
    delay(5000); // Run motor for 500ms
    digitalWrite(Step_Motor, LOW); // Turn off motor
}

void Screw(){
    printf("Screw moving\n");
    digitalWrite(Screw_Motor, LOW); // Turn on motor
    delay(5000); // Run motor for 500ms
    digitalWrite(Screw_Motor, HIGH); // Turn off motor
}

void LoadRaise(){
    printf("Loader should be raising\n");
    digitalWrite(Loader_Raise, HIGH); // Activate raise (assuming active-low)
    delay(5000); // Raise loader for 500ms
    digitalWrite(Loader_Raise, LOW); // Deactivate raise
}

void LoadLow(){
    printf("Loader should be lowering\n");
    digitalWrite(Loader_Lower, HIGH); // Activate lower (assuming active-low)
    delay(5000); // Lower loader for 500ms
    digitalWrite(Loader_Lower, LOW); // Deactivate lower
}

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
// Function to send integer values as a comma-separated string
void uart_direction_Write(int fd, int x) {
    char data[50];  // Buffer to hold formatted string

    // Format integer into string and ensure it's null-terminated
    snprintf(data, sizeof(data), "%.2d", x);
    int bytes_written = write(fd, data, strlen(data));

    if (bytes_written < 0) {
        perror("Error formatting data");
        return;
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

int main(void)
{
    if (wiringPiSetupGpio() == -1) { // Use BCM pin numbering
        printf("WiringPi setup failed!\n");
        return 1;
    }

    setup();
    int uart_fd = open(UART_PORT, O_RDWR | O_NOCTTY | O_NDELAY); // Open UART port

    if (uart_fd == -1) {
        perror("UART Open Error");
        return 1;
    }

    configure_uart(uart_fd);  // Configure the UART
    while(1){
        // Test the motors and loader
        uart_direction_Write(uart_fd, 1);
        Brush();
        uart_direction_Write(uart_fd, 2);
        Step();
        uart_direction_Write(uart_fd, 3);
        Screw();
        uart_direction_Write(uart_fd, 4);
        LoadRaise();
        uart_direction_Write(uart_fd, 5);
        LoadLow();
        uart_direction_Write(uart_fd, 6);
        printf("Sent data!");
        delay(200);
    }
}
