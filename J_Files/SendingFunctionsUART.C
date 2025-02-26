#include <stdio.h>  // Standard C library
#include <stdlib.h> // Standard C library
#include <fcntl.h>  // File control options
#include <unistd.h> // For read, write, and sleep functions
#include <termios.h> // For UART configuration
#include <string.h>  // String manipulation
#include <errno.h>   // Error handling

#define UART_PORT "/dev/ttyAMA0"  // Change this to the correct UART port for Raspberry Pi 5

// Function to configure UART
void configure_uart(int uart_fd) {
    struct termios options;
    tcgetattr(uart_fd, &options); // Get current UART settings

    // Set baud rate to 9600 bps
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);

    // Set UART parameters: 8 data bits, no parity, 1 stop bit
    options.c_cflag &= ~PARENB;    // No parity
    options.c_cflag &= ~CSTOPB;    // 1 stop bit
    options.c_cflag &= ~CSIZE;     // Clear current data size settings
    options.c_cflag |= CS8;        // 8 data bits
    options.c_cflag |= (CLOCAL | CREAD); // Enable receiver and ignore modem control lines

    // Disable canonical mode, echo, and signal characters for raw data input
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY); // Disable software flow control
    options.c_oflag &= ~OPOST; // Raw output mode

    // Apply the configuration immediately
    tcsetattr(uart_fd, TCSANOW, &options);
}

// Function to write data to UART
void uart_write(int fd, const char *data) {
    int len = strlen(data);
    int bytes_written = write(fd, data, len);

    if (bytes_written < 0) {
        perror("UART write failed");
    } else {
        printf("Sent: %s\n", data);
    }
}

int main() {
    int uart_fd = open(UART_PORT, O_RDWR | O_NOCTTY); // Open UART for reading and writing

    if (uart_fd == -1) {
        perror("Unable to open UART");
        return -1;
    }

    configure_uart(uart_fd); // Set UART settings

    int condition1 = 1; // Example condition for sending '1'
    int condition2 = 0; // Example condition for sending '2'
    int condition3 = 0; // Example condition for sending '3'

    // Check conditions and send corresponding number
    
    if (condition1) {
        uart_write(uart_fd, "1");
    } else if (condition2) {
        uart_write(uart_fd, "2");
    } else if (condition3) {
        uart_write(uart_fd, "3");
    } else {
        printf("No condition met. No data sent.\n");
    }
    
    //sleep(1); // Delay of 1 second

    
   

    close(uart_fd); // Close the UART connection
    return 0;
}
