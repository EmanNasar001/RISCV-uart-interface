# RISC-V ACT Framework — UART Interface Implementation

Coding challenge submission for the **RISC-V ACT Framework Enablement and M-Mode Firmware Validation** mentorship (LFX Mentorship, Summer 2026).

**Author:** Eman Nasar — UET Lahore  
**GitHub:** [github.com/EmanNasar001/RISCV-uart-interface](https://github.com/EmanNasar001/RISCV-uart-interface)



## What This Does

A C program that:
- Configures a UART interface on Linux using the `termios` API (115200 baud, 8N1)
- Transmits a test message over UART
- Receives incoming data using non-blocking I/O with `select()` and a 3-second timeout
- Prints received data to the console
- Handles errors gracefully (bad device path, permission denied, read/write failures)

## Files

| File | Description |
|------|-------------|
| `uart_interface.c` | Main C source code |
| `Makefile` | Build helper |
| `README.md` | This file |

---

## Step 1 — Install Dependencies (WSL Ubuntu)

```bash
sudo apt update && sudo apt install -y build-essential socat
```

---

## Step 2 — Clone and Build

```bash
git clone https://github.com/EmanNasar001/RISCV-uart-interface
cd RISCV-uart-interface
gcc -Wall -Wextra -std=c11 -g -o uart_interface uart_interface.c
```

Confirm build succeeded:
```bash
ls -lh uart_interface
```

---

## Step 3 — Test Without Hardware (WSL Virtual Loopback)

You need **3 terminal windows** open.

### Terminal 1 — Create virtual serial port pair
```bash
socat -d -d pty,raw,echo=0 pty,raw,echo=0
```
Note the two port numbers printed, for example:
```
PTY is /dev/pts/4
PTY is /dev/pts/5
```
**Leave this terminal running.**

### Terminal 2 — Run the UART program
```bash
cd RISCV-uart-interface
./uart_interface /dev/pts/4
```
The program will wait up to 3 seconds for incoming data.

### Terminal 3 — Simulate board response (within 3 seconds!)
```bash
echo "ACK from fake RISC-V board" > /dev/pts/5
```


## UART Parameters

Edit these defines at the top of uart_interface.c:

| Define | Default | Options |
|--------|---------|---------|
| BAUD_RATE | B115200 | B9600, B57600, B115200 |
| DATA_BITS | CS8 | CS5, CS6, CS7, CS8 |
| USE_PARITY | 0 (none) | 0 = none, 1 = enable |
| ODD_PARITY | 0 (even) | 0 = even, 1 = odd |
| TWO_STOP_BITS | 0 (1 stop bit) | 0 = 1, 1 = 2 |
| RX_TIMEOUT_SEC | 3 | any positive integer |

---

|
