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

#include "embeddedCommErrors.hpp"
#include "embeddedCommChecksum.hpp"
#include "embeddedCommConstants.hpp"

class GenericSlave {
public:
	GenericSlave();

	// Pass both memory and receive buffer memory spaces which must be already reserved.
	void initialize(uint8_t *memory, uint32_t memorySize, uint8_t *rBuffer, uint32_t rBufferSize);

	// Handle received byte according to EmbeddedComm protocol.
	void writeHandler(uint8_t receivedByte);
	
	// Handle received stop signal (end of current transaction) according to EmbeddedComm protocol.
	void stopHandler();

	// Handle byte request according to EmbeddedComm protocol. Return byte to send out.
	uint8_t readHandler();

private:
	
	// Possible transfer states
	enum transferState {
		STATE_IDLE,             // Idle, ready for new transfer
		STATE_R_ADDRESS,        // Receiving memory address
		STATE_R_DATA,           // Receiving data bytes
		STATE_T_DATA,           // Transmitting data
	};

	// Set error flag in slave's status byte.
	inline void setErrorFlag(uint8_t error);

	// Put data from receive buffer to memory at location pointed by memoryAddress. 
	void writeMemory();

	// Set memoryAddress to first four bytes of receive buffer.
	void writeMemoryAddress();

	// Change state and set member variables to match it.
	void changeTransferState(transferState newState);
	
	// Put byte to write buffer on first free spot.
	void writeBuffer(uint8_t byte);

	uint8_t *memory; // Pointer to device memory reserved for slave's memory.
	uint8_t *rBuffer; // Pointer to device memory reserved for slave's receive buffer.
	uint8_t *statusByte; // &memory[STATUS_BYTE_ADDRESS]
	uint8_t *tChecksumByte; // &memory[TCHECKSUM_BYTE_ADDRESS]
	uint32_t rBufferSize; // Bytes
	uint32_t memorySize; // Bytes
	volatile uint32_t memoryAddress; // Current memory address used for write/read operations.
	volatile uint32_t byteCounter; // Helper value used during reads and writes to keep track of number of bytes.
	volatile transferState currentState; // Current transfer state, different action will take place in according to this value.
	volatile uint8_t checksum; // Currently calculated checksum.
};

// Set given error flag in status register
void GenericSlave::setErrorFlag(uint8_t error) {
	(*statusByte) |= error;
} 
