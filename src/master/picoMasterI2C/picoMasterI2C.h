#pragma once

#include <hardware/i2c.h>
#include <hardware/gpio.h>

#include "I2CConductor.hpp"


class picoMasterI2C : public I2CConductor<uint8_t> {
public:
	picoMasterI2C(uint8_t scl, uint8_t sda, i2c_inst_t *i2c);

protected:
	int readBytes(uint8_t &slaveAddress, uint8_t *byteArray, uint32_t numberOfBytes) override;
	int writeBytes(uint8_t &slaveAddress, uint8_t *byteArray, uint32_t numberOfBytes) override;

private:
	i2c_inst_t *i2cInstance;
};
