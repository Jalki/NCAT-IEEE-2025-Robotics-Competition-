#include <stdio.h> //Standard C library
#include <stdlib.h> //Standard C librarby
#include <wiringPi.h> //Wiring Pi
#include <fcntl.h>
#include <unistd.h> //Used to convert Strings to characters
#include <termios.h> //
#include <string.h>
#include <errno.h>
#include <unistd.h> // For sleep function
#include <sys/time.h> //System time

#define TRIG 4  // GPIO pin for trigger
#define ECHO 5  // GPIO pin for echo

#define UART_PORT "/dev/ttyAMA0"  // Change this to your serial port

char gyro[256];
char newGyro[256];
char accel[256];
char newaccel[256];


// Function to configure UART
void configure_uart(int uart_fd) {
    struct termios options;
    tcgetattr(uart_fd, &options); // Get current serial port settings

    // Set baud rate
    cfsetispeed(&options, B9600); // Input baud rate
    cfsetospeed(&options, B9600); // Output baud rate

    // 8 data bits, no parity, 1 stop bit
    options.c_cflag &= ~PARENB;    // No parity bit
    options.c_cflag &= ~CSTOPB;    // 1 stop bit
    options.c_cflag &= ~CSIZE;     // Clear data size bits
    options.c_cflag |= CS8;        // 8 data bits

    // Enable receiver and transmitter
    options.c_cflag |= (CLOCAL | CREAD);

    // Set the port options
    tcsetattr(uart_fd, TCSANOW, &options);
}

long getMicrotime() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    return currentTime.tv_sec * 1000000 + currentTime.tv_usec;
}

// Function to get distance from ultrasonic sensor
float getDistance() {
    long startTime, stopTime;
    float distance;
    
    // Send trigger pulse
    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);
    
    // Wait for echo to go high
    while (digitalRead(ECHO) == LOW);
    startTime = getMicrotime();
    
    // Wait for echo to go low
    while (digitalRead(ECHO) == HIGH);
    stopTime = getMicrotime();
    
    // Calculate distance in cm
    distance = (stopTime - startTime) * 0.0343 / 2;
    
    return distance;
}
// Function to write data to UART
void uart_write(int fd, const char *data) {
    int len = strlen(data);
    int bytes_written = write(fd, data, len);
    if (bytes_written < 0) {
        perror("Write error");
    } else {
        printf("Sent: %s\n", data);
    }
}

void motor_control() {
    float gyro_data, new_gyro_data;
    gyro_data = atof(gyro); // Convert gyro data to float
    new_gyro_data = atof(newGyro); // Convert newGyro data to float
    sleep(1); // Delay of 1 second
}

// Function to read data from UART
void uart_read(int fd) {
    int n = 0;
    n = read(fd, gyro, sizeof(gyro) - 1);
    gyro[n] = '\0';
    printf("This is the gyro point: %s\n", gyro);
    while (1) {
        n = read(fd, newGyro, sizeof(newGyro) - 1);
        if (n > 0) {
            newGyro[n] = '\0'; // Null-terminate the received data
            printf("Received: %s\n", newGyro);
        } else if (n < 0) {
            perror("Read error");
            break;
       }
    }
}

int main() {
    int uart_fd = open(UART_PORT, O_RDWR | O_NOCTTY);

    int work = 0;

    if (uart_fd == -1) {
        perror("Unable to open UART");
        return -1;
    }

    if (wiringPiSetup() == -1) {
        printf("Failed to initialize wiringPi\n");
        return 1;
    }

    configure_uart(uart_fd);

    int condition = 1; // Example condition for sending '1'

    // Check conditions and send corresponding number
    while (1) {
        float distance = getDistance();
        printf("Distance: %.2f cm\n", distance);
        delay(250); // Wait quarter a second before next measurement
        if (distance > 2.5){
            while(condition){
                switch (condition)
                {
                case 1:
                    condition = 2;
                    uart_write(uart_fd, "1");
                    sleep(.5);
                case 2:
                    condition = 3;
                    uart_write(uart_fd, "2");
                    sleep(0.5);
                case 3:
                    condition = 4;
                    uart_write(uart_fd, "3");
                case 4:
                    condition = 5;
                    uart_write(uart_fd, "4");
                    sleep(0.5);
                case 5:
                    condition = 6;
                    uart_write(uart_fd, "5");
                    sleep(0.5);
                case 6:
                    condition = 6;
                    uart_write(uart_fd, "6");
                    sleep(0.5);
                case 7:
                    condition = 8;
                    uart_write(uart_fd, "7");
                    sleep(0.5);
                case 8:
                    condition = 9;
                    uart_write(uart_fd, "8");
                    sleep(0.5);
                case 9:
                    condition = 10;
                    uart_write(uart_fd, "9");
                    sleep(0.5);
                case 10: 
                    condition = 0;
                    uart_write(uart_fd, "10");
                    sleep(0.5);
                case 0:
                    condition = 1;
                    uart_write(uart_fd, "0");
                    sleep(0.5);
                default:
                    break;
                }
            }
        }else{
            printf("YOU DONE FUCKED UP!");
        }
    }

    // Read data
    uart_read(uart_fd);

    close(uart_fd);
    return 0;
}
