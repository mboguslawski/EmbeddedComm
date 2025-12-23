/*
GenericSlave.cpp

GenericSlave class' logic implementation.

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

#include "GenericSlave.hpp"
#include <stdlib.h>
#include <iostream>
GenericSlave::GenericSlave():
	memory(nullptr),
	backupBuffer(nullptr),
	backupBufferSize(0),
	memorySize(0),
	memoryAddress(0),
	dataLength(0),
	byteCounter(0),
	checksum(0),
	statusValue(Ok)
{}

void GenericSlave::initialize(uint8_t *memory, uint32_t memorySize) {
	this->memory = memory;
	this->memorySize = memorySize;
}

void GenericSlave::enableMemBackups(uint8_t *backupBuffer, uint32_t backupBufferSize) {
	this->backupBuffer = backupBuffer;
	this->backupBufferSize = backupBufferSize;
}

void GenericSlave::process() {
	if (restoreBackupPending) {
		restoreBackup();
	}
}

// Handle all logic related to slave receiving byte from master.
void GenericSlave::writeHandler(uint8_t receivedByte) {
	
	// Handle received byte according to protocol
	// its meaning is known based on byteCounter, 
	// which tracks how many bytes where transferred since last reset.

	// Received byte is a part of transfer size declared by master.
	if (byteCounter < SLAVE_ADDRESS_SIZE) {
		receiveDataLength(receivedByte);
		checksum = calculateChecksumIt(checksum, receivedByte);

	// Received byte is a part of memory address,
	// which is the first address from which master will read or to which master will write data.
	} else if (byteCounter < SLAVE_ADDRESS_SIZE*2) {
		receiveMemoryAddress(receivedByte);
		checksum = calculateChecksumIt(checksum, receivedByte);

	// Received byte data master writes to slave.
	} else if (byteCounter < SLAVE_ADDRESS_SIZE*2 + dataLength) {
		receiveData(receivedByte);
		checksum = calculateChecksumIt(checksum, receivedByte);

	// Received byte is checksum
	} else if (byteCounter == SLAVE_ADDRESS_SIZE*2 + dataLength) {
		if (checksum != receivedByte) {
			setStatusValueFlag(ErrDataCorrupted, &statusValue);
		}

	// At this point only read request is acceptable (to read status).
	} else {
		setStatusValueFlag(ErrInvalidWrite, &statusValue);
	}

	byteCounter++;
}

uint8_t GenericSlave::readHandler() {
	uint8_t out_byte = 0x0;

	// At this point of transfer master should write dataLength and memorySize
	if (byteCounter < SLAVE_ADDRESS_SIZE*2) {
		setStatusValueFlag(ErrInvalidRead, &statusValue);
	
	// Return byte read from memory
	} else if (byteCounter < SLAVE_ADDRESS_SIZE*2 + dataLength) {
		uint32_t readAddress = byteCounter - SLAVE_ADDRESS_SIZE*2 + memoryAddress;

		if (readAddress >= memorySize) {
			setStatusValueFlag(ErrMemoryOutOfRange, &statusValue);
			byteCounter++;
			return out_byte;
		}

		if (statusValue == Ok) {
			out_byte = memory[readAddress];
		}
	
	// Return checksum byte
	} else if (byteCounter == SLAVE_ADDRESS_SIZE*2 + dataLength) {
		out_byte = checksum;
	
	// Return status byte
	} else {
		if ( (statusValue & ErrDataCorrupted) && (backupBuffer != nullptr) ) {
			restoreBackupPending = true;
			setStatusValueFlag(Busy, &statusValue);
		}

		out_byte = (uint8_t)statusValue;

		reset();
		return out_byte;
	} 

	byteCounter++;
	checksum = calculateChecksumIt(checksum, out_byte);
	return out_byte;
}

void GenericSlave::reset() {
	if (restoreBackupPending) {
		return;
	}
	
	byteCounter = 0;
	dataLength = 0;
	memoryAddress = 0;
	checksum = 0;
	statusValue = Ok;
}

void GenericSlave::restoreBackup() {
	memcpy(&memory[memoryAddress], backupBuffer, dataLength);
	restoreBackupPending = false;
	reset();
}

void GenericSlave::receiveDataLength(uint8_t receivedByte) {
	dataLength |= (uint32_t)receivedByte << (byteCounter);
	
	if ( (byteCounter == SLAVE_ADDRESS_SIZE-1)  && (backupBuffer != nullptr) && (dataLength > backupBufferSize) ){
		setStatusValueFlag(ErrBackupBufferOverflow, &statusValue);
	}
}

void GenericSlave::receiveMemoryAddress(uint8_t receivedByte) {
	memoryAddress |= (uint32_t)receivedByte << (byteCounter - SLAVE_ADDRESS_SIZE);

	if ( (byteCounter == SLAVE_ADDRESS_SIZE*2-1) && (memoryAddress + dataLength >= memorySize) ) {
		setStatusValueFlag(ErrMemoryOutOfRange, &statusValue);
	}
}

void GenericSlave::receiveData(uint8_t receivedByte) {
	// Do not process if any errors occurred.
	if (statusValue != Ok) {
		return;
	}
	
	uint32_t writeAddress = memoryAddress + byteCounter - SLAVE_ADDRESS_SIZE*2;

	if (writeAddress >= memorySize) {
		setStatusValueFlag(ErrMemoryOutOfRange, &statusValue);
		byteCounter++;
		return;
	}

	if (backupBuffer != nullptr) {
		if (writeAddress - memoryAddress >= backupBufferSize) {
			setStatusValueFlag(ErrBackupBufferOverflow, &statusValue);
			byteCounter++;
			return;
		}
			
		backupBuffer[writeAddress - memoryAddress] = memory[writeAddress];
	}
		
	memory[writeAddress] = receivedByte;
}
