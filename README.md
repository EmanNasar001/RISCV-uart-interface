# RISC-V ACT Framework - UART Interface

A UART interface in C using termios API for the RISC-V ACT Framework mentorship challenge.

## Build
gcc -Wall -Wextra -std=c11 -g -o uart_interface uart_interface.c

## Run
./uart_interface /dev/pts/4

## Test (WSL No Hardware)
socat -d -d pty,raw,echo=0 pty,raw,echo=0
./uart_interface /dev/pts/4 & sleep 1 && echo "ACK" > /dev/pts/5

## Author
Eman Nasar
