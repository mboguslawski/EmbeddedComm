/*
embeddedCommErrors.hpp

Definition of possible EmbeddedComm slave status register values.

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
	
using StatusValue = uint8_t;

// All statuses have values which are power of 2, so they can be used as flags
enum CommStatus {
	// Do not use this value as 0 could be return by some read functions as default value, when read does not succeed 
	NotUsed = 0,

	// Memory address is not valid, falls outside memory range.
	ErrMemoryOutOfRange = 1,

	// Slave's backup buffer overflowed, once backups are enabled maximum write size must not exceed backup buffer size.
	ErrBackupBufferOverflow = 2,
		
	// Master wanted to read data, but did not write dataLen or/and memoryAddress previously.
	ErrInvalidRead = 4,

	// Master wanted to write data when it was supposed to read status byte.
	ErrInvalidWrite = 8,

	// Checksum send by master does not match checksum calculated based on received data.
	ErrDataCorrupted = 16,

	// Slave is not ready for read/write requests (eg. memory backup needs to be restored). 
	Busy = 32,
	
	// Status indicates no errors
	Ok = 128
};

// Sets given flag in statusValue
inline void setStatusValueFlag(CommStatus flag, volatile StatusValue *statusValue) {
	if (flag == Ok) {
		(*statusValue) &= Ok;
		return;
	}

	// Clear Ok flag and set given flag
	(*statusValue) &= ~Ok;
	(*statusValue) |= flag;
}
