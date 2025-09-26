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
