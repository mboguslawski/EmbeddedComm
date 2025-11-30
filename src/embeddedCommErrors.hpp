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

#include <string>

namespace EmbeddedComm {
	enum CommStatus {
		OK = 0x0, 					// No errors
		ErrMemoryOutOfRange = 0x1,	// Memory address is not valid, falls outside memory range.
		ErrRBufferOverflow = 0x2,		// Slave's write buffer is overflowing, reduce packet size or make slave's write buffer bigger.
		ErrInvalidAction = 0x4,		// Master performed action that is consistent with protocol.
		ErrDataCorrupted = 0x8		// Checksum send by master does not match data.
	};

	inline std::string statusToString(CommStatus status) {
		switch (status) {
		
		case OK:
			return "OK";
		case ErrMemoryOutOfRange:
			return "ERROR: memory out of range";
		case ErrRBufferOverflow:
			return "ERROR: slave's read buffer overflow";
		case ErrInvalidAction:
			return "ERROR: invalid action";
		case ErrDataCorrupted:
			return "ERROR: data corrupted";
		}
	}
};
