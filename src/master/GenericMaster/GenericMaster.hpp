#pragma once

#include <string.h>

#include "../../common/embeddedCommErrors.hpp"
#include "../../common/embeddedCommChecksum.hpp"
#include "../../common/embeddedCommConstants.hpp"

template <typename slaveInfo>
class GenericMaster {
public:
	GenericMaster();
	
	int write(slaveInfo &sinfo, uint32_t memoryAddress, uint8_t *data, uint32_t writeSize);
	int read(slaveInfo &sinfo, uint32_t memoryAddress, uint8_t *buffer, uint32_t readSize);
	inline int readStatus(slaveInfo &sinfo, uint8_t* statusByte);
	inline int readTChecksum(slaveInfo &sinfo, uint8_t* tChecksumByte);

protected:
	virtual int writeBytes(slaveInfo &sinfo, uint8_t *bytes, uint32_t numberOfBytes) = 0;
	virtual int readBytes(slaveInfo &sinfo, uint8_t *bytes, uint32_t numberOfBytes) = 0;
};

template <typename slaveInfo>
GenericMaster<slaveInfo>::GenericMaster() {}

template <typename slaveInfo>
int GenericMaster<slaveInfo>::write(slaveInfo &sinfo, uint32_t memoryAddress, uint8_t *data, uint32_t writeSize) {
	// Data size, four bytes for address and one byte for checksum.
	const uint32_t messageBufferSize = writeSize + EmbeddedComm::SLAVE_ADDRESS_SIZE + EmbeddedComm::CHECKSUM_SIZE;
	uint8_t messageBuffer[messageBufferSize];

	// Instrument's memory address
	memcpy(messageBuffer, &memoryAddress, EmbeddedComm::SLAVE_ADDRESS_SIZE);
	// Data
	if (data != NULL) {
		memcpy(messageBuffer + EmbeddedComm::SLAVE_ADDRESS_SIZE, data, writeSize);
	}
	// Attach checksum.
	messageBuffer[messageBufferSize - 1] = EmbeddedComm::calculateChecksum(messageBuffer, messageBufferSize-1);

	return writeBytes(sinfo, messageBuffer, messageBufferSize);
}

template <typename slaveInfo>
int GenericMaster<slaveInfo>::read(slaveInfo &sinfo, uint32_t memoryAddress, uint8_t *buffer, uint32_t readSize) {
	// Write address and transfer size
	int res = write(sinfo, memoryAddress, NULL, 0);
	if (res < 0) {
		return res;
	}

	// Read data from slave into read buffer
	return readBytes(sinfo, buffer, readSize);
}

template <typename slaveInfo>
int GenericMaster<slaveInfo>::readStatus(slaveInfo &sinfo, uint8_t* statusByte) {
	return read(sinfo, EmbeddedComm::DEFAULT_STATUS_BYTE_ADDRESS, statusByte, EmbeddedComm::STATUS_SIZE);
}

template <typename slaveInfo>
int GenericMaster<slaveInfo>::readTChecksum(slaveInfo &sinfo, uint8_t* tChecksumByte) {
	return read(sinfo, EmbeddedComm::DEFAULT_T_CHECKSUM_BYTE_ADDRESS, tChecksumByte, EmbeddedComm::CHECKSUM_SIZE);
}
