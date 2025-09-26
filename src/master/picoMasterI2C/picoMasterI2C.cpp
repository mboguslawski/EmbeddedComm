/*
picoMasterI2C.cpp

Implementation of picoMasterI2C class' logic.

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

#include "picoMasterI2C.hpp"

picoMasterI2C::picoMasterI2C(uint8_t scl, uint8_t sda, i2c_inst_t *i2c, uint32_t i2cFreqKhz):
	i2cInstance(i2c)
{
	gpio_init(sda);
	gpio_set_function(sda, GPIO_FUNC_I2C);
	gpio_pull_up(sda);

	gpio_init(scl);
	gpio_set_function(scl, GPIO_FUNC_I2C);
	gpio_pull_up(scl);

	i2c_init(i2c, i2cFreqKhz * 1000);
}

int picoMasterI2C::readBytes(uint8_t &slaveAddress, uint8_t *byteArray, uint32_t numberOfBytes) {
	return i2c_read_blocking(i2cInstance, slaveAddress, byteArray, numberOfBytes, false);	
}

int picoMasterI2C::writeBytes(uint8_t &slaveAddress, uint8_t *byteArray, uint32_t numberOfBytes) {
	return i2c_write_blocking(i2cInstance, slaveAddress, byteArray, numberOfBytes, false);	
}
