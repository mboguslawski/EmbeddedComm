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
#include <iostream>

#include "CommStatus.hpp"
#include "CommChecksum.hpp"
#include "CommConstants.hpp"

// Template implementation allows flexibility for child classes in defining slave information types.
template <typename slaveInfo>
class GenericMaster {
public:
	GenericMaster();
	
	// Write bytes to (pointed by sinfo parameter) slave's memory starting with given address.
	// Keep in mind that write to slave is limited by its receive buffer capacity (minus one byte to account for checksum).
	// As return value, pass code returned by some hardware-specific write function from child class. 
	StatusValue write(slaveInfo &sinfo, uint32_t memoryAddress, uint8_t *data, uint32_t writeSize, bool falseChecksum = false);
	
	// Read bytes from (pointed by sinfo parameter) slave's memory starting with given address,
	// load data into buffer. Ensure buffer has atleast readSize bytes.
	// As return value, pass code returned by some hardware-specific read function from child class. 
	StatusValue read(slaveInfo &sinfo, uint32_t memoryAddress, uint8_t *buffer, uint32_t readSize);
	
	// Read zero data bytes from slave, to get status value
	inline StatusValue readStatus(slaveInfo &sinfo);

protected:
	// Some hardware-specific function used to write bytes to slave.
	virtual int writeBytes(slaveInfo &sinfo, uint8_t *bytes, uint32_t numberOfBytes) = 0;

	// Some hardware-specific function used to read bytes from slave.
	virtual int readBytes(slaveInfo &sinfo, uint8_t *bytes, uint32_t numberOfBytes) = 0;
};

template <typename slaveInfo>
GenericMaster<slaveInfo>::GenericMaster() {}

template <typename slaveInfo>
StatusValue GenericMaster<slaveInfo>::write(slaveInfo &sinfo, uint32_t memoryAddress, uint8_t *data, uint32_t writeSize, bool falseChecksum) {
	// four bytes for data length + four bytes for address + writeSize bytes for data + one byte for checksum.
	const uint32_t messageBufferSize = writeSize + SLAVE_ADDRESS_SIZE * 2 + CHECKSUM_SIZE;
	uint8_t messageBuffer[messageBufferSize];

	// Data length
	memcpy(messageBuffer, &writeSize, SLAVE_ADDRESS_SIZE);

	// Memory address
	memcpy(messageBuffer + SLAVE_ADDRESS_SIZE, &memoryAddress, SLAVE_ADDRESS_SIZE);
	
	// Data
	if (data != NULL) {
		memcpy(&messageBuffer[SLAVE_ADDRESS_SIZE*2], data, writeSize);
	}

	// Attach checksum.
	messageBuffer[messageBufferSize - 1] = calculateChecksum(messageBuffer, messageBufferSize-1);
	if (falseChecksum) {
		messageBuffer[messageBufferSize - 1]++;
	}

	if (writeBytes(sinfo, messageBuffer, messageBufferSize) < 0) {
		printf("write failed\n");
		return 0;
	}

	StatusValue status;
	if (readBytes(sinfo, &status, 1) < 0) {
		printf("read failed\n");

		return 0;
	}

	return status;
}

template <typename slaveInfo>
StatusValue GenericMaster<slaveInfo>::read(slaveInfo &sinfo, uint32_t memoryAddress, uint8_t *buffer, uint32_t readSize) {
	const uint32_t messageBufferSize = SLAVE_ADDRESS_SIZE * 2;
	uint8_t messageBuffer[messageBufferSize];

	memcpy(messageBuffer, &readSize, SLAVE_ADDRESS_SIZE);
	memcpy(messageBuffer + SLAVE_ADDRESS_SIZE, &memoryAddress, SLAVE_ADDRESS_SIZE);

	writeBytes(sinfo, messageBuffer, messageBufferSize);

	// Read data from slave into read buffer
	readBytes(sinfo, buffer, readSize);

	uint8_t receivedChecksum;
	readBytes(sinfo, &receivedChecksum, 1);

	uint8_t checksum = calculateChecksum(messageBuffer, messageBufferSize);

	if (calculateChecksumAppend(buffer, readSize, checksum) != receivedChecksum) {
		StatusValue status;
		readBytes(sinfo, &status, 1);
		return ErrDataCorrupted;
	}

	StatusValue status;
	if (readBytes(sinfo, &status, 1) < 0) {
		printf("read failed\n");

		return 0;
	}

	return status;
}

template <typename slaveInfo>
StatusValue GenericMaster<slaveInfo>::readStatus(slaveInfo &sinfo) {
	uint8_t dummy;
	return read(sinfo, 0, &dummy, 1);
}
