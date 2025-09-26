/*
embeddedCommConstants.hpp

Definitions of constant values used in EmbeddedComm library code.

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

namespace EmbeddedComm {
	constexpr uint32_t SLAVE_ADDRESS_SIZE = 4; // Size of slave's memory addresses in bytes.
	
	constexpr uint32_t STATUS_SIZE = 1; // Slave's status size in bytes.
	constexpr uint32_t DEFAULT_STATUS_BYTE_ADDRESS = 0x0; // Default slave's status byte address in its memory.
	
	constexpr uint32_t CHECKSUM_SIZE = 1; // Checksum size in bytes.
	constexpr uint32_t DEFAULT_T_CHECKSUM_BYTE_ADDRESS = 0x1; // Default slave's tChecksum byte address in its memory.
};
