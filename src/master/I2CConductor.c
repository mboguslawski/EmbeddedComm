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
	uint32_t message_buff_size = data_size + instrument->memory_address_size * 2 + 1;
    uint8_t message_buffer[message_buff_size];

	// Instrument's memory address
	memcpy(message_buffer, &memory_address, instrument->memory_address_size);
    // Transfer size
    memcpy(message_buffer + instrument->memory_address_size, &data_size, instrument->memory_address_size);
    // Accual data
    memcpy(message_buffer + instrument->memory_address_size * 2, data, data_size);
    // Calculate checksum and send it as last byte.
    message_buffer[message_buff_size - 1] = calc_checksum(message_buffer, message_buff_size - 1);

    return i2c_write_blocking(instrument->i2c, instrument->i2c_address, message_buffer, message_buff_size, false);
}

int i2c_conductor_read(i2c_instrument_info_t *instrument, uint32_t memory_address, uint8_t *data, uint32_t data_size) {
    // Address and transfer size will be written to i2c slave, both are the same  size.
    uint32_t write_buffer_size = instrument->memory_address_size * 2;
    uint8_t write_buffer[write_buffer_size];

    // Prepare write buffer
	memcpy(write_buffer, &memory_address, instrument->memory_address_size);
	memcpy(write_buffer + instrument->memory_address_size, &data_size, instrument->memory_address_size);

    // Write address and transfer size
    int res = i2c_write_blocking(instrument->i2c, instrument->i2c_address, write_buffer, write_buffer_size, false);
    if (res < 0) {
        return res;
    }

    uint8_t w_checksum = calc_checksum(write_buffer, write_buffer_size);

    // Read data from slave into read buffer
    uint8_t read_buffer[data_size + 1];
    res = i2c_read_blocking(instrument->i2c, instrument->i2c_address, read_buffer, data_size + 1, false);
    if (res < 0) {
        return res;
    }

    // Transfer data from read buffer only if checksum matches
    if (calc_checksum_append(read_buffer, data_size, w_checksum) == read_buffer[data_size]) {
        memcpy(data, read_buffer, data_size);
    }

    return data_size;
}

int i2c_conductor_read_instrument_status(i2c_instrument_info_t *instrument, status_register_t *status) {
    status_register_t ret = 0;
    return i2c_read_blocking(instrument->i2c, instrument->i2c_address, status, 1, false);
}
