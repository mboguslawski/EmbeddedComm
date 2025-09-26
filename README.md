<!-- 
Documentation for EmbeddedComm project.

Copyright (C) 2025 Mateusz Bogusławski, E: mateusz.boguslawski@ibnet.pl
-->

# EmbeddedComm

**Memory-Based Master-Slave Communication Protocol for Microcontrollers**  

Supports one master and multiple slaves for memory read/write operations.
Allows communication between different microcontrollers through protocol abstraction, provided they use the same underlying hardware transfer protocol (e.g., a Raspberry Pi Pico can communicate with an Arduino or ESP module via I²C).

---

### Table of Contents

- [1. Slave Memory](#1-slave-memory)  
  - [1.1 Memory Layout](#11-memory-layout)  
  - [1.2 Status Byte](#12-status-byte)  
  - [1.3 Transmit Checksum Byte](#13-transmit-checksum-byte)  
- [2. Protocol Specification](#2-protocol-specification)  
  - [2.1 Master → Slave: Memory Write](#21-master--slave-memory-write)  
  - [2.2 Master → Slave: Memory Read](#22-master--slave-memory-read)
- [3. Hardware-Specific Implementations](#3-hardware-specific-implementations)
  - [RPi Pico (RP2040, RP2350 with I2C)](#311-rpi-pico-rp2040-rp2350)

---

## 1. Slave Memory

Slave memory is passed by user during slave initialization, so its size can be chosen depending on needs.

### 1.1 Memory layout

| Address | Description |
|---------|-------------|
| 0x0 | Status Byte |
| 0x1 | Transmit Checksum Byte |
| 0x2 | User defined byte
| ... | ... |
| 0xn | User defined byte|

To store one byte of memory, three bytes of memory must be reserved. The maximum amount of reserved memory depends on the programmer’s implementation and the hardware specifications.

### 1.2 Status Byte

Reading the status byte from slave's memory allows master to determine if last write/read operation succeeded. 

**Status byte flags:**

| Bit number | Flag | Description | Possible Fix |
|------------|------|-------------|----------|
| **0** | **ErrMemoryOutOfRange** | Master wants to read/write to a memory location that is not in slave memory. | Make sure both the memory address and number of transferred bytes match the slave's memory size. |
| **1** | **ErrRBufferOverflow** | Slave's write buffer is overflowing. | Reduce the packet size or increase the slave's write buffer. |
| **2** | **ErrInvalidAction** | Master performed an action that is inconsistent with the protocol. | Ensure all master operations follow the protocol. |
| **3** | **ErrDataCorrupted** | Checksum sent by master does not match the data. | Verify the checksum calculation and ensure data integrity during transmission. |
| **4-7** | **Not used** | - | - |

### 1.3 Transmit Checksum Byte
This byte, stored in the slave's memory, contains the checksum calculated from the last master read operation. It allows the master to verify the integrity of the received data.

## 2. Protocol Specification

### 2.1 Master → Slave: Memory Write

The master writes data to the slave's memory using the following frame format:

| Field       | Size        | Description                                 |
|------------|------------|---------------------------------------------|
| Address    | 4 Bytes    | Target memory address in the slave          |
| Data       | n Bytes    | Data to be written (can be 0 bytes)        |
| Checksum   | 1 Byte     | Error-detection checksum                     |

> **Note:** Data length can be zero. In this case, only the address is written to the slave.

Data received by the slave is stored in its receive buffer. The last byte received is the checksum. The number of bytes the master can write to the slave is limited by the size of the slave’s buffer, which is user-defined (note that the checksum byte is also stored in the buffer). Once the checksum is verified, the received data is transferred from the receive buffer to the slave’s memory.


**Write Sequence Diagram:**

```text
Master                               Slave
  |                                    |
  |---[ Address | Data | Checksum ]--->|
  |                                    |
```

---

### 2.2 Master → Slave: Memory Read

Memory read is a two-step process:

#### Step 1: Write Read Address

The master writes the target memory address to the slave:

| Field       | Size        | Description                     |
|------------|------------|---------------------------------|
| Address    | 4 Bytes    | Memory address to read          |
| Checksum   | 1 Byte     | Error-detection checksum        |

> **Note:** This is a standard master write operation with no data payload.

#### Step 2: Read Data from Slave

After sending the address, the master reads the data from the slave:

| Field       | Size        | Description                     |
|------------|------------|---------------------------------|
| Data       | n Bytes    | Data read from slave memory     |


The slave transfers data to the master starting from the memory location specified by the most recently written address. The internal address pointer auto-increments after each byte transfer, allowing sequential reads. The total number of bytes transferred is limited only by the size of the slave’s memory.

**Read Sequence Diagram:**

```text
Master                        Slave
  |                             |
  |---[ Address | Checksum ]--->|
  |                             |
  |                             |
  |<----------[ Data ]----------|
  |                             |
```

## 3. Hardware-specific implementations.

### 3.1 I2C
#### 3.1.1 RPi Pico (RP2040, RP2350)
- **Master implementation:** [picoMasterI2C](examples/picoMasterI2C/)
- **Slave implementation:** [picoSlaveI2C](examples/picoSlaveI2C/)
