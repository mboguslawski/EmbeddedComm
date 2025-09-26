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

	statusByte = memory + 0;
	tChecksumByte = memory + 1;
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

	if (newAddress >= memorySize) {
		setErrorFlag(CommStatus::ErrMemoryOutOfRange);
		return;
	}

	memoryAddress = newAddress;
}

void GenericSlave::changeTransferState(transferState newState) {
	switch (newState) {
	
	case STATE_IDLE:
		if (currentState == STATE_R_DATA) {
			writeMemory();
		
		} else if (currentState == STATE_T_DATA) {
			(*tChecksumByte) = checksum;
		}

		break;

	case STATE_R_ADDRESS:
		memoryAddress = 0;
		byteCounter = 0;
		break;
	
	case STATE_R_DATA:
		checksum = calc_checksum(rBuffer, ADDRESS_SIZE);
		writeMemoryAddress();
		byteCounter = 0;
		break;

	case STATE_T_DATA:
		byteCounter = 0;
		checksum = 0;
		break;
	}

	currentState = newState;
}

// Writes received byte to memory/write buffer and checks for errors.
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
		checksum = calc_checksum_it(checksum, rBuffer[byteCounter-1]);
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
	if ( (currentState == STATE_R_ADDRESS) && (byteCounter == ADDRESS_SIZE)) {
		changeTransferState(STATE_R_DATA);
	}
}

uint8_t GenericSlave::readHandler() {
	uint8_t out_byte = 0x0;

	if (currentState == STATE_IDLE) {
		changeTransferState(STATE_T_DATA);
	}
	
	uint32_t idx = memoryAddress + byteCounter;

	if (idx >= memorySize) {
		setErrorFlag(CommStatus::ErrMemoryOutOfRange);
		return out_byte;
	}

	out_byte = memory[idx];
	byteCounter++;

	if (idx == 0) {
		(*statusByte) = CommStatus::OK;
	}

	checksum = calc_checksum_it(checksum, out_byte);

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
