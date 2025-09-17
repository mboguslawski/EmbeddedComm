#ifndef I2CCONDUCTOR_H
#define I2CCONDUCTOR_H

#include <hardware/i2c.h>
#include <hardware/gpio.h>
#include <string.h>

#include "i2c_orchestra_errors.h"
#include "i2c_orchestra_checksum.h"

typedef struct {
	uint8_t memory_address_size;
	uint8_t i2c_address;
	i2c_inst_t *i2c;
} i2c_instrument_info_t;

void i2c_conductor_init(uint8_t scl, uint8_t sda, i2c_inst_t *i2c);
int i2c_conductor_write(i2c_instrument_info_t *instrument, uint32_t memory_address, uint8_t *data, uint32_t data_size);
int i2c_conductor_read_instrument_status(i2c_instrument_info_t *instrument, status_register_t *status);
int i2c_conductor_read(i2c_instrument_info_t *instrument, uint32_t memory_address, uint8_t *data, uint32_t data_size);

#endif
