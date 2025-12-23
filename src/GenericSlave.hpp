/*
GenericSlave.hpp

GenericSlave class implements EmbeddedComm slave logic.
This class is not dependent on any hardware and specific protocol such as I2C/SPI
and therefore cannot be directly used alone.

Copyright (C) 2025 Mateusz Bogusławski, E: mateusz.boguslawski@ibnet.pl

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see https://www.gnu.org/licenses/.
*/

#pragma once

#include <string.h>
#include <cstdint>

#include "CommStatus.hpp"
#include "CommChecksum.hpp"
#include "CommConstants.hpp"

class GenericSlave {
public:
	GenericSlave();

	// Pass pointer to buffer which will be used as memory.
	void initialize(uint8_t *memory, uint32_t memorySize);

	// Enabling backups will restore previous data when corrupted during transfer.
	// Pass pointer to buffer which will be used to store memory backup.
	// After memory backups are enabled, maximum write size is restricted by backupBufferSIze
	void enableMemBackups(uint8_t *backupBuffer, uint32_t backupBufferSize);

	// Handle received byte according to EmbeddedComm protocol.
	void writeHandler(uint8_t receivedByte);

	// Handle byte request according to EmbeddedComm protocol. Return byte to send out.
	uint8_t readHandler();

	// Need to be called frequentlly, manages potentially time-consuming task (eg. moving data from rBuffer to memory).
	void process();

private:
	// Resets internal values to prepare for next transfer
	void reset();

	// Restore backup from backupBuffer
	void restoreBackup();

	void receiveMemoryAddress(uint8_t receivedByte);

	void receiveDataLength(uint8_t receivedByte);

	void receiveData(uint8_t receivedByte);

	uint8_t *memory; // Pointer to device memory reserved for slave's memory.
	uint8_t *backupBuffer; // Pointer to device memory reserved for slave's receive buffer.
	uint32_t backupBufferSize; // Bytes
	uint32_t memorySize; // Bytes
	volatile uint32_t memoryAddress; // Current memory address used for write/read operations.
	volatile uint32_t dataLength;
	volatile uint32_t byteCounter; // Helper value used during reads and writes to keep track of number of bytes.
	volatile uint8_t checksum;
	volatile StatusValue statusValue;
	volatile bool restoreBackupPending;
};
