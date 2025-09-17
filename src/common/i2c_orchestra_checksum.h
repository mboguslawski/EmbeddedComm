#ifndef I2C_ORCHESTRA_CHECKSUM_H
#define I2C_ORCHESTRA_CHECKSUM_H

// Calculate CRC8 (see https://en.wikipedia.org/wiki/Cyclic_redundancy_check)
// using iterative method.
inline uint8_t calc_checksum_it(uint8_t checksum, uint8_t data) {
	checksum ^= data;
    for (uint8_t i = 0; i < 8; i++) {
        if (checksum & 0x80)
            checksum = (checksum << 1) ^ 0x07;
        else
            checksum <<= 1;
    }

	return checksum;
}

inline uint8_t calc_checksum_append(uint8_t *data, uint32_t size, uint8_t start_val) {
	for (uint32_t i = 0; i < size; i++) {
		start_val = calc_checksum_it(start_val, data[i]);
	}

	return start_val;
}

inline uint8_t calc_checksum(uint8_t *data, uint32_t size) {
	return calc_checksum_append(data, size, 0);
}

#endif
