#ifndef I2CINSTRUMENT_H
#define I2CINSTRUMENT_H

#include <hardware/i2c.h>
#include <hardware/gpio.h>
#include <pico/i2c_slave.h>
#include <string.h>

#include "i2c_orchestra_errors.h"

void i2c_instrument_init(uint8_t scl, uint8_t sda, i2c_inst_t *i2c, uint8_t i2c_address, uint8_t *memory, uint32_t memory_size, uint8_t *mem_buffer, uint32_t mem_buffer_size);
uint8_t i2c_instrument_get_status(i2c_inst_t *i2c);

#endif
