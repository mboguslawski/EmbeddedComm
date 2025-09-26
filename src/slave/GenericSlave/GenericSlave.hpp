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

#include "../../common/embeddedCommErrors.hpp"
#include "../../common/embeddedCommChecksum.hpp"
#include "../../common/embeddedCommConstants.hpp"

class GenericSlave {
public:
	GenericSlave();

	void initialize(uint8_t *memory, uint32_t memorySize, uint8_t *rBuffer, uint32_t rBufferSize);
	void writeHandler(uint8_t receivedByte);
	void stopHandler();
	uint8_t readHandler();

private:
	
	// Possible transfer states
	enum transferState {
		STATE_IDLE,             // Idle, ready for new transfer
		STATE_R_ADDRESS,        // Receiving memory address
		STATE_R_DATA,           // Receiving data bytes
		STATE_T_DATA,           // Transmitting data
	};

	inline void setErrorFlag(uint8_t error);

	void writeMemory();
	void writeMemoryAddress();
	void changeTransferState(transferState newState);
	void writeBuffer(uint8_t byte);

	uint8_t *memory;
	uint8_t *rBuffer;
	uint8_t *statusByte;
	uint8_t *tChecksumByte;
	uint32_t rBufferSize;
	uint32_t memorySize;
	volatile uint32_t memoryAddress;
	volatile uint32_t byteCounter;
	volatile transferState currentState;
	volatile uint8_t checksum;
};

// Set given error flag in status register
void GenericSlave::setErrorFlag(uint8_t error) {
	(*statusByte) |= error;
} 
