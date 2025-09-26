#pragma once

#include <string.h>
#include <cstdint>

#include "../../common/embeddedCommErrors.hpp"
#include "../../common/embeddedCommChecksum.hpp"

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

	static const uint32_t ADDRESS_SIZE = 4; // Memory address size in bytes.

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
