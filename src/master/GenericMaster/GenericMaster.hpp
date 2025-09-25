#pragma once

#include <string.h>

#include "../../common/i2c_orchestra_errors.h"
#include "../../common/i2c_orchestra_checksum.h"

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
	const uint32_t messageBufferSize = writeSize + 4 + 1;
	uint8_t messageBuffer[messageBufferSize];

	// Instrument's memory address
	memcpy(messageBuffer, &memoryAddress, 4);
	// Data
	if (data != NULL) {
		memcpy(messageBuffer + 4, data, writeSize);
	}
	// Attach checksum.
	messageBuffer[messageBufferSize - 1] = calc_checksum(messageBuffer, messageBufferSize-1);

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
	return read(sinfo, 0, statusByte, 1);
}

template <typename slaveInfo>
int GenericMaster<slaveInfo>::readTChecksum(slaveInfo &sinfo, uint8_t* tChecksumByte) {
	return read(sinfo, 0, tChecksumByte, 1);
}
