#ifndef PICOSLAVEI2C_H
#define PICOSLAVEI2C_H

#include <pico/stdlib.h>
#include <pico/i2c_slave.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>

#include "GenericSlave.hpp"

void initPicoSlaveI2C(uint8_t scl, uint8_t sda, i2c_inst_t *i2c, uint8_t i2c_address, uint8_t *memory, uint32_t memorySize, uint8_t *rBuffer, uint32_t rBufferSze);

#endif
