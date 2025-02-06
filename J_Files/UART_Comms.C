#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <errno.h>

#define UART_PORT "/dev/ttyAMA0"  // Change this to your serial port

int main() {
    int uart_fd = open(UART_PORT, O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart_fd == -1) {
        perror("Unable to open UART");
        return -1;
    }

    // Configure UART settings
    struct termios options;
    tcgetattr(uart_fd, &options); // Get current serial port settings

    // Set baud rate
    cfsetispeed(&options, B115200); // Input baud rate
    cfsetospeed(&options, B115200); // Output baud rate

    // 8 data bits, no parity, 1 stop bit
    options.c_cflag &= ~PARENB;    // No parity bit
    options.c_cflag &= ~CSTOPB;    // 1 stop bit
    options.c_cflag &= ~CSIZE;     // Clear data size bits
    options.c_cflag |= CS8;        // 8 data bits

    // Enable receiver and disable transmitter (read-only mode)
    options.c_cflag |= (CLOCAL | CREAD);

    // Set the port options
    tcsetattr(uart_fd, TCSANOW, &options);

    // Read data from UART
    char buffer[256];
    int n = 0;
    while (1) {
        n = read(uart_fd, buffer, sizeof(buffer)-1);
        if (n > 0) {
            buffer[n] = '\0'; // Null-terminate the received data
            printf("Received: %s\n", buffer);
        } else if (n < 0) {
            perror("Read error");
            break;
        }
    }

    close(uart_fd);
    return 0;
}
