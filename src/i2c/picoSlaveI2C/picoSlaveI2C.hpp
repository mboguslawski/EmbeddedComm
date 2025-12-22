/*
picoSlaveI2C.hpp

Implementation of GenericSlave class for raspberry pi pico (rp2040 and rp2350),
with I2C protocol.

Copyright (C) 2025 Mateusz Bogusławski, E: mateusz.boguslawski@ibnet.pl

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see https://www.gnu.org/licenses/.
*/

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

	// Initialize I2C interface and slave logic. Ensure that declared memory and receive buffer sizes match real ones.
	void initialize(uint8_t scl, uint8_t sda, i2c_inst_t *i2c, uint32_t i2cFreqKHz, uint8_t i2c_address, uint8_t *memory, uint32_t memorySize);

private:
	i2c_inst_t *i2cInstance;
};

#endif
