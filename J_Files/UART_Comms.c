#include <stdio.h>    // Standard I/O functions
#include <stdlib.h>   // Standard C library
#include <wiringPi.h> // Wiring Pi
#include <fcntl.h>    // File control
#include <unistd.h>   // POSIX API (sleep, read, write)
#include <termios.h>  // Terminal I/O
#include <string.h>   // String functions
#include <errno.h>    // Error handling
#include <sys/time.h> // System time functions

#define UART_PORT "/dev/ttyAMA0" // Change this to your actual serial port

char gyro[256];
char newGyro[256];
char accel[256];
char newaccel[256];

char buffer[256];

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
void uart_direction_Write(int fd, float x, float z, float rotation) {
    char data[50];  // Buffer to hold formatted string
    snprintf(data, sizeof(data), "%.2f,%.2f,%.2f\n", x, z, rotation);
    
    int bytes_written = write(fd, data, strlen(data));
    if (bytes_written < 0) {
        perror("UART Write Error");
    } else {
        printf("Sent: %s", data);
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
    int uart_fd = open(UART_PORT, O_RDWR | O_NOCTTY);
    if (uart_fd == -1) {
        perror("Unable to open UART");
        return -1;
    }

    configure_uart(uart_fd);

    float x = 0.34, z = 0.78, rotation = 90.12;

    while (1) {
        uart_read(uart_fd);
        uart_direction_Write(uart_fd, x, z, rotation);
        sleep(1);
    }

    close(uart_fd);
    return 0;
}