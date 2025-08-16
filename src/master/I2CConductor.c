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
	uint32_t message_buff_size = data_size + instrument->memory_address_size;
    uint8_t message_buffer[message_buff_size];

	// Instrument's memory address
	memcpy(message_buffer, &memory_address, instrument->memory_address_size);
    // Accual data
    memcpy(message_buffer + instrument->memory_address_size, data, data_size);

    return i2c_write_blocking(instrument->i2c, instrument->i2c_address, message_buffer, message_buff_size, false);
}
