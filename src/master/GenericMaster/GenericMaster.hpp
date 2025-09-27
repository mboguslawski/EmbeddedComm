/*
GenericMaster.hpp

GenericMaster class implements EmbeddedComm master logic.
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

#include "../../common/embeddedCommErrors.hpp"
#include "../../common/embeddedCommChecksum.hpp"
#include "../../common/embeddedCommConstants.hpp"

// Template implementation allows flexibility for child classes in defining slave information types.
template <typename slaveInfo>
class GenericMaster {
public:
	GenericMaster();
	
	// Write bytes to (pointed by sinfo parameter) slave's memory starting with given address.
	// Keep in mind that write to slave is limited by its receive buffer capacity (minus one byte to account for checksum).
	// As return value, pass code returned by some hardware-specific write function from child class. 
	int write(slaveInfo &sinfo, uint32_t memoryAddress, uint8_t *data, uint32_t writeSize);
	
	// Read bytes from (pointed by sinfo parameter) slave's memory starting with given address,
	// load data into buffer. Ensure buffer has atleast readSize bytes.
	// As return value, pass code returned by some hardware-specific read function from child class. 
	int read(slaveInfo &sinfo, uint32_t memoryAddress, uint8_t *buffer, uint32_t readSize);
	
	// Read one byte at status address using read method.
	inline int readStatus(slaveInfo &sinfo, uint8_t* statusByte);

	// Read one byte at transmit checksum address using read method.
	inline int readTChecksum(slaveInfo &sinfo, uint8_t* tChecksumByte);

protected:
	// Some hardware-specific function used to write bytes to slave.
	virtual int writeBytes(slaveInfo &sinfo, uint8_t *bytes, uint32_t numberOfBytes) = 0;

	// Some hardware-specific function used to read bytes from slave.
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
