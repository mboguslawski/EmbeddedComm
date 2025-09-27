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

using namespace EmbeddedComm;

GenericSlave::GenericSlave():
	memory(nullptr),
	rBuffer(nullptr),
	statusByte(nullptr),
	tChecksumByte(nullptr),
	rBufferSize(0),
	memorySize(0),
	memoryAddress(0),
	byteCounter(0),
	currentState(STATE_IDLE),
	checksum(0)
{}

void GenericSlave::initialize(uint8_t *memory, uint32_t memorySize, uint8_t *rBuffer, uint32_t rBufferSize) {
	this->memory = memory;
	this->memorySize = memorySize;
	this->rBuffer = rBuffer;
	this->rBufferSize = rBufferSize;

	statusByte = memory + DEFAULT_STATUS_BYTE_ADDRESS;
	tChecksumByte = memory + DEFAULT_T_CHECKSUM_BYTE_ADDRESS;
}

// Move received data from master to memory.
void GenericSlave::writeMemory() {

	// Copy data from rBuffer to memory
	if ( (currentState == STATE_R_DATA) && ((*statusByte) == CommStatus::OK) ) {
		// Check data integrity (compare checksum)
		// Last byte send by master is checksum
		if (checksum != rBuffer[byteCounter - 1]) {
			setErrorFlag(CommStatus::ErrDataCorrupted);
			return;
		}
		
		memcpy(memory + memoryAddress, rBuffer, byteCounter-1);
	}
}

// Copies first four bytes from wbuffer to memoryAddress
// and sets error is address is not valid.
void GenericSlave::writeMemoryAddress() {
	// Receive memory address (can be multiple bytes long)
	uint32_t newAddress = *((uint32_t*)rBuffer);

	// Ensure that address does not point outside memory.
	if (newAddress >= memorySize) {
		setErrorFlag(CommStatus::ErrMemoryOutOfRange);
		return;
	}

	memoryAddress = newAddress;
}

void GenericSlave::changeTransferState(transferState newState) {
	switch (newState) {
	
	case STATE_IDLE:
		// Receicing data finished, move data from receive buffer to memory.
		if (currentState == STATE_R_DATA) {
			writeMemory();
		
		// Sending data finished, set transmit checksum byte value.
		} else if (currentState == STATE_T_DATA) {
			(*tChecksumByte) = checksum;
		}

		break;

	case STATE_R_ADDRESS:
		// Set values to prepeare for receiving memory address value.
		memoryAddress = 0;
		byteCounter = 0;
		break;
	
	case STATE_R_DATA:
		// Checksum is calculated from both received memory and data.
		// Calculate and save checksum from address part.
		checksum = calculateChecksum(rBuffer, EmbeddedComm::SLAVE_ADDRESS_SIZE);
		writeMemoryAddress(); // Set memory address value.
		byteCounter = 0;
		break;

	case STATE_T_DATA:
		// Set values to prepeare for transmitting data.
		byteCounter = 0;
		checksum = 0;
		break;
	}

	currentState = newState;
}

// Writes received byte to write buffer and checks for errors.
void GenericSlave::writeBuffer(uint8_t byte) {
	
	// Check for overflow in rBuffer and/or write outside memory. 
	if ( (byteCounter >= rBufferSize) || (memoryAddress + byteCounter >= memorySize) ) {
		setErrorFlag(CommStatus::ErrMemoryOutOfRange);
		return;
	}

	rBuffer[byteCounter] = byte;

	// As last received byte will be checksum, calculate checksum from previously received byte,
	// so the received checksum itself is not used in calculating checksum on slave's side.
	if ( (currentState == STATE_R_DATA) && (byteCounter != 0)) {
		checksum = calculateChecksumIt(checksum, rBuffer[byteCounter-1]);
	}

	byteCounter++;
}

// Handle all logic related to slave receiving byte from master.
void GenericSlave::writeHandler(uint8_t received_byte) {
	// Decide where to save receivied byte according to current state.
	if (currentState == STATE_IDLE) {
		changeTransferState(STATE_R_ADDRESS);
	}

	writeBuffer(received_byte);

	// Check if memory address is fully received in buffer
	if ( (currentState == STATE_R_ADDRESS) && (byteCounter == SLAVE_ADDRESS_SIZE)) {
		changeTransferState(STATE_R_DATA);
	}
}

uint8_t GenericSlave::readHandler() {
	uint8_t out_byte = 0x0;

	if (currentState == STATE_IDLE) {
		// Move to data transmit state.
		changeTransferState(STATE_T_DATA);
	}
	
	uint32_t idx = memoryAddress + byteCounter;

	// Ensure that read location does not point outside memory region.
	if (idx >= memorySize) {
		setErrorFlag(CommStatus::ErrMemoryOutOfRange);
		return out_byte;
	}

	out_byte = memory[idx];
	byteCounter++;

	if (idx == DEFAULT_STATUS_BYTE_ADDRESS) {
		// Status byte work in the read-to-clear manner.
		(*statusByte) = CommStatus::OK;
	}

	// Include each byte in checksum calculation.
	checksum = calculateChecksumIt(checksum, out_byte);

	return out_byte;
}

void GenericSlave::stopHandler() {
	
	// Stop signal should not occur in STATE_R_ADDEREE and STATE_IDLE states.
	if ( (currentState == STATE_R_ADDRESS) || (currentState == STATE_IDLE) ) {
		setErrorFlag(CommStatus::ErrInvalidAction);
	}
	
	// Master couldn't read 0 bytes.
	if ( (currentState == STATE_T_DATA) && (byteCounter == 0) ) {
		setErrorFlag(CommStatus::ErrInvalidAction);
	}

	changeTransferState(STATE_IDLE);
}
