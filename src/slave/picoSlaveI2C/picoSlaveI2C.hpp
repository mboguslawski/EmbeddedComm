#ifndef PICOSLAVEI2C_H
#define PICOSLAVEI2C_H

#include <pico/stdlib.h>
#include <pico/i2c_slave.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>

#include "GenericSlave.hpp"

class picoSlaveI2C : public GenericSlave {
public:
	picoSlaveI2C();
	~picoSlaveI2C();

	void initialize(uint8_t scl, uint8_t sda, i2c_inst_t *i2c, uint32_t i2cFreqKHz, uint8_t i2c_address, uint8_t *memory, uint32_t memorySize, uint8_t *rBuffer, uint32_t rBufferSize);

private:
	i2c_inst_t *i2cInstance;
};

#endif
