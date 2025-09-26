#pragma once

namespace EmbeddedComm {
	constexpr uint32_t SLAVE_ADDRESS_SIZE = 4; // Size of slave's memory addresses in bytes.
	
	constexpr uint32_t STATUS_SIZE = 1; // Slave's status size in bytes.
	constexpr uint32_t DEFAULT_STATUS_BYTE_ADDRESS = 0x0; // Default slave's status byte address in its memory.
	
	constexpr uint32_t CHECKSUM_SIZE = 1; // Checksum size in bytes.
	constexpr uint32_t DEFAULT_T_CHECKSUM_BYTE_ADDRESS = 0x1; // Default slave's tChecksum byte address in its memory.
};
