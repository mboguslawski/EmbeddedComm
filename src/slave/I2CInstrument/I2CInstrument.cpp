#include "I2CInstrument.h"

I2CInstrument::I2CInstrument():
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

void I2CInstrument::initialize(uint8_t *memory, uint32_t memorySize, uint8_t *rBuffer, uint32_t rBufferSize) {
	this->memory = memory;
	this->memorySize = memorySize;
	this->rBuffer = rBuffer;
	this->rBufferSize = rBufferSize;

	statusByte = memory + 0;
	tChecksumByte = memory + 1;
}

// Move received data from master to memory.
void I2CInstrument::writeMemory() {

	// Copy data from rBuffer to memory
	if ( (currentState == STATE_R_DATA) && ((*statusByte) == I2C_O_OK) ) {
		// Check data integrity (compare checksum)
		// Last byte send by master is checksum
		if (checksum != rBuffer[byteCounter - 1]) {
			setErrorFlag(I2C_O_ERR_DATA_CORRUPTED);
			return;
		}
		
		memcpy(memory + memoryAddress, rBuffer, byteCounter-1);
	}
}

// Copies first four bytes from wbuffer to memoryAddress
// and sets error is address is not valid.
void I2CInstrument::writeMemoryAddress() {
	// Receive memory address (can be multiple bytes long)
	uint32_t newAddress = *((uint32_t*)rBuffer);

	if (newAddress >= memorySize) {
		setErrorFlag(I2C_O_ERR_MEM_OUT_OF_RANGE);
		return;
	}

	memoryAddress = newAddress;
}

void I2CInstrument::changeTransferState(transferState newState) {
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
void I2CInstrument::writeBuffer(uint8_t byte) {
	
	// Check for overflow in rBuffer and/or write outside memory. 
	if ( (byteCounter >= rBufferSize) || (memoryAddress + byteCounter >= memorySize) ) {
		setErrorFlag(I2C_O_ERR_MEM_OUT_OF_RANGE);
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
void I2CInstrument::writeHandler(uint8_t received_byte) {
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

uint8_t I2CInstrument::readHandler() {
	uint8_t out_byte = 0x0;

	if (currentState == STATE_IDLE) {
		changeTransferState(STATE_T_DATA);
	}
	
	uint32_t idx = memoryAddress + byteCounter;

	if (idx >= memorySize) {
		setErrorFlag(I2C_O_ERR_MEM_OUT_OF_RANGE);
		return out_byte;
	}

	out_byte = memory[idx];
	byteCounter++;

	if (idx == 0) {
		(*statusByte) = I2C_O_OK;
	}

	checksum = calc_checksum_it(checksum, out_byte);

	return out_byte;
}

void I2CInstrument::stopHandler() {
	
	// Stop signal should not occur in STATE_R_ADDEREE and STATE_IDLE states.
	if ( (currentState == STATE_R_ADDRESS) || (currentState == STATE_IDLE) ) {
		setErrorFlag(I2C_O_ERR_INVALID_ACTION);
	}
	
	// Master couldn't read 0 bytes.
	if ( (currentState == STATE_T_DATA) && (byteCounter == 0) ) {
		setErrorFlag(I2C_O_ERR_INVALID_ACTION);
	}

	changeTransferState(STATE_IDLE);
}
