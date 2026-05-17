CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g
TARGET  = uart_interface
SRC     = uart_interface.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Build successful"

clean:
	rm -f $(TARGET)
