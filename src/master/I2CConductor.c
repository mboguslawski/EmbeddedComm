#include "I2CConductor.h"

void i2c_conductor_init(uint8_t scl, uint8_t sda, i2c_inst_t *i2c) {
	gpio_init(sda);
	gpio_set_function(sda, GPIO_FUNC_I2C);
	gpio_pull_up(sda);

	gpio_init(scl);
	gpio_set_function(scl, GPIO_FUNC_I2C);
	gpio_pull_up(scl);

	i2c_init(i2c, 1*1000*1000);
}

int i2c_conductor_write(i2c_instrument_info_t *instrument, uint32_t memory_address, uint8_t *data, uint32_t data_size) {
	// Data size and four bytes for address.
	const uint32_t message_buff_size = data_size + 4;
	uint8_t message_buffer[message_buff_size];

	// Instrument's memory address
	memcpy(message_buffer, &memory_address, 4);
	// Data
	memcpy(message_buffer + 4, data, data_size);

	return i2c_write_blocking(instrument->i2c, instrument->i2c_address, message_buffer, message_buff_size, false);
}

int i2c_conductor_read(i2c_instrument_info_t *instrument, uint32_t memory_address, uint8_t *data, uint32_t data_size) {
	// Write address and transfer size
	int res = i2c_write_blocking(instrument->i2c, instrument->i2c_address, &memory_address, 4, false);
	if (res < 0) {
		return res;
	}

	// Read data from slave into read buffer
	return i2c_read_blocking(instrument->i2c, instrument->i2c_address, data, data_size, false);
}

int i2c_conductor_read_instrument_status(i2c_instrument_info_t *instrument, uint8_t *status) {
	return i2c_conductor_read(instrument, 0, status, 1);
}
