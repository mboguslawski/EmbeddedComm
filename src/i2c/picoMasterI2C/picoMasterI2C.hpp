/*
picoMasterI2C.hpp

Implementation of GenericMaster class for raspberry pi pico (rp2040 and rp2350),
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

#pragma once

#include <hardware/i2c.h>
#include <hardware/gpio.h>

#include "GenericMaster.hpp"

// 7-bit I2C address is used to identify slave, so 8-bit int type is used
// to pass slave's information (its address).
class picoMasterI2C : public GenericMaster<uint8_t> {
public:
	picoMasterI2C(uint8_t scl, uint8_t sda, i2c_inst_t *i2c, uint32_t i2cFreqKHz);

protected:
	// Hardware-specific function to read bytes from slave via I2C.
	// Pass pico c sdk i2c_read() function result as return value.
	int readBytes(uint8_t &slaveAddress, uint8_t *byteArray, uint32_t numberOfBytes) override;
	
	// Hardware-specific function to write bytes from slave via I2C.
	// Pass pico c sdk i2c_write() function result as return value.
	int writeBytes(uint8_t &slaveAddress, uint8_t *byteArray, uint32_t numberOfBytes) override;

private:
	i2c_inst_t *i2cInstance; // i2c0 or i2c1
};
