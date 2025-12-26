/*
embeddedCommChecksum.hpp

Checksum calculating functions used by both EmbeddedComm slave and master devices.

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

#include <cstdint>

// Calculate CRC8 (see https://en.wikipedia.org/wiki/Cyclic_redundancy_check)
// using iterative method.
inline uint8_t calculateChecksumIt(uint8_t checksum, uint8_t data) {
	checksum ^= data;
    for (uint8_t i = 0; i < 8; i++) {
        if (checksum & 0x80)
            checksum = (checksum << 1) ^ 0x07;
        else
            checksum <<= 1;
    }

	return checksum;
}

// Calculate CRC8 (see https://en.wikipedia.org/wiki/Cyclic_redundancy_check)
// with defined start value.
inline uint8_t  calculateChecksumAppend(uint8_t *data, uint32_t size, uint8_t startValue) {
	for (uint32_t i = 0; i < size; i++) {
		startValue = calculateChecksumIt(startValue, data[i]);
	}

	return startValue;
}

// Calculate CRC8 (see https://en.wikipedia.org/wiki/Cyclic_redundancy_check)
// with 0 as start value.
inline uint8_t calculateChecksum(uint8_t *data, uint32_t size) {
	return calculateChecksumAppend(data, size, 0);
}

