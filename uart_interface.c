#define _GNU_SOURCE
/**
 * uart_interface.c - RISC-V ACT Framework UART Implementation
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BAUD_RATE       B115200
#define DATA_BITS       CS8
#define USE_PARITY      0
#define ODD_PARITY      0
#define TWO_STOP_BITS   0
#define TX_MESSAGE      "Hello from RISC-V ACT UART Test!\r\n"
#define RX_TIMEOUT_SEC  3
#define RX_BUF_SIZE     256

int uart_open(const char *device_path) {
    int fd = open(device_path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        fprintf(stderr, "[ERROR] Cannot open '%s': %s\n", device_path, strerror(errno));
        if (errno == ENOENT)
            fprintf(stderr, "       Device path does not exist.\n");
        else if (errno == EACCES || errno == EPERM)
            fprintf(stderr, "       Permission denied. Try: sudo ./uart_interface %s\n", device_path);
        return -1;
    }
    if (!isatty(fd)) {
        fprintf(stderr, "[ERROR] '%s' is not a TTY device.\n", device_path);
        close(fd);
        return -1;
    }
    printf("[INFO]  Opened '%s' (fd=%d)\n", device_path, fd);
    return fd;
}

int uart_configure(int fd) {
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        fprintf(stderr, "[ERROR] tcgetattr failed: %s\n", strerror(errno));
        return -1;
    }
    cfsetispeed(&tty, BAUD_RATE);
    cfsetospeed(&tty, BAUD_RATE);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |=  DATA_BITS;
    tty.c_cflag |=  CREAD | CLOCAL;
    if (TWO_STOP_BITS) tty.c_cflag |=  CSTOPB;
    else               tty.c_cflag &= ~CSTOPB;
    if (USE_PARITY) {
        tty.c_cflag |= PARENB;
        if (ODD_PARITY) tty.c_cflag |=  PARODD;
        else            tty.c_cflag &= ~PARODD;
    } else {
        tty.c_cflag &= ~PARENB;
    }
    tty.c_cflag &= ~CRTSCTS;
    cfmakeraw(&tty);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_oflag &= ~OPOST;
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 0;
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        fprintf(stderr, "[ERROR] tcsetattr failed: %s\n", strerror(errno));
        return -1;
    }
    tcflush(fd, TCIOFLUSH);
    printf("[INFO]  UART configured: 115200 8%s%s\n",
           USE_PARITY ? (ODD_PARITY ? "O" : "E") : "N",
           TWO_STOP_BITS ? "2" : "1");
    return 0;
}

int uart_transmit(int fd, const char *message) {
    size_t total = strlen(message), written = 0;
    printf("[TX]    Sending %zu bytes: \"%s\"\n", total, message);
    while (written < total) {
        ssize_t n = write(fd, message + written, total - written);
        if (n < 0) {
            if (errno == EINTR) continue;
            fprintf(stderr, "[ERROR] write() failed: %s\n", strerror(errno));
            return -1;
        }
        written += (size_t)n;
    }
    tcdrain(fd);
    printf("[TX]    Transmitted %zu bytes successfully.\n", written);
    return 0;
}

int uart_receive(int fd) {
    char buf[RX_BUF_SIZE];
    fd_set read_fds;
    struct timeval timeout;
    int total_received = 0;
    printf("[INFO]  Waiting up to %d second(s) for incoming data...\n", RX_TIMEOUT_SEC);
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);
        timeout.tv_sec  = RX_TIMEOUT_SEC;
        timeout.tv_usec = 0;
        int ret = select(fd + 1, &read_fds, NULL, NULL, &timeout);
        if (ret < 0) {
            if (errno == EINTR) continue;
            fprintf(stderr, "[ERROR] select() failed: %s\n", strerror(errno));
            return -1;
        }
        if (ret == 0) { printf("[INFO]  Receive timeout - no more data.\n"); break; }
        if (FD_ISSET(fd, &read_fds)) {
            ssize_t n = read(fd, buf, sizeof(buf) - 1);
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
                fprintf(stderr, "[ERROR] read() failed: %s\n", strerror(errno));
                return -1;
            }
            if (n == 0) { printf("[INFO]  Device closed (EOF).\n"); break; }
            buf[n] = '\0';
            printf("[RX]    %zd byte(s) received: \"%s\"\n", n, buf);
            total_received += (int)n;
        }
    }
    return total_received;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <device_path>\nExample: %s /dev/pts/2\n", argv[0], argv[0]);
        return EXIT_FAILURE;
    }
    int fd = uart_open(argv[1]);
    if (fd < 0) return EXIT_FAILURE;
    int exit_code = EXIT_SUCCESS;
    if (uart_configure(fd) < 0) { exit_code = EXIT_FAILURE; goto cleanup; }
    if (uart_transmit(fd, TX_MESSAGE) < 0) { exit_code = EXIT_FAILURE; goto cleanup; }
    int bytes = uart_receive(fd);
    if (bytes < 0) { exit_code = EXIT_FAILURE; goto cleanup; }
    printf("[INFO]  Total bytes received: %d\n", bytes);
cleanup:
    close(fd);
    printf("[INFO]  Device '%s' closed.\n", argv[1]);
    return exit_code;
}
