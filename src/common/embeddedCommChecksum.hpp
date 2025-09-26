#pragma once

namespace EmbeddedComm {

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

}
