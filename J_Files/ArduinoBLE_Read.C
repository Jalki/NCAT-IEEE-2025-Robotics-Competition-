#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>

#define SERIAL_PORT "/dev/serial0"  // Serial port for Raspberry Pi UART
#define BAUD_RATE B115200           // Baud rate for communication

int main() {
    // Open the serial port
    int fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd == -1) {
        perror("Unable to open serial port");
        return 1;
    }

    // Configure the serial port (set baud rate, parity, etc.)
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        perror("Error getting serial attributes");
        close(fd);
        return 1;
    }

    // Set baud rate, 8 data bits, no parity, 1 stop bit
    cfsetospeed(&tty, BAUD_RATE);
    cfsetispeed(&tty, BAUD_RATE);
    tty.c_cflag |= (CLOCAL | CREAD);  // Enable receiver and set local mode
    tty.c_cflag &= ~CSIZE;            // Clear the current character size mask
    tty.c_cflag |= CS8;               // 8 data bits
    tty.c_cflag &= ~PARENB;           // No parity
    tty.c_cflag &= ~CSTOPB;           // 1 stop bit
    tty.c_cflag &= ~CRTSCTS;          // Disable hardware flow control
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Disable software flow control
    tty.c_iflag &= ~ICANON;           // Raw input mode
    tty.c_iflag &= ~ECHO;             // Disable echo
    tty.c_iflag &= ~ECHOE;            // Disable erasure
    tty.c_iflag &= ~ISIG;             // Disable signal generation
    tty.c_oflag &= ~OPOST;            // Raw output mode
    tty.c_cc[VMIN] = 1;               // Read at least one byte
    tty.c_cc[VTIME] = 10;             // Timeout after 1 second

    // Apply the configuration
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("Error setting serial attributes");
        close(fd);
        return 1;
    }

    // Buffer to read the incoming data (6 floats = 24 bytes)
    unsigned char data[24];

    while (1) {
        // Read data from the serial port
        ssize_t bytesRead = read(fd, data, sizeof(data));
        if (bytesRead > 0) {
            // Extract the 6 floats from the data (24 bytes)
            float accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z;
            memcpy(&accel_x, &data[0], sizeof(float));
            memcpy(&accel_y, &data[4], sizeof(float));
            memcpy(&accel_z, &data[8], sizeof(float));
            memcpy(&gyro_x, &data[12], sizeof(float));
            memcpy(&gyro_y, &data[16], sizeof(float));
            memcpy(&gyro_z, &data[20], sizeof(float));

            // Print the extracted data
            printf("Accel: (%.2f, %.2f, %.2f) Gyro: (%.2f, %.2f, %.2f)\n",
                   accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z);
        } else {
            // If no data is available or error occurred, continue reading
            if (bytesRead < 0) {
                perror("Error reading from serial port");
            }
        }
    }

    // Close the serial port
    close(fd);
    return 0;
}
