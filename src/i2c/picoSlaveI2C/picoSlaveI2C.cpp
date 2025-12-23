/*
picoSlaveI2C.hpp

picoSlaveI2C class' logic implementation.

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

#include "picoSlaveI2C.hpp"

// The I2C interrupt handler requires a specific function signature and cannot be a non-static
// class method (because of the implicit 'this' pointer). 
// This implementation provides a workaround: we map each interrupt source to a corresponding 
// picoSlaveI2C object. Since the number of objects is limited by the number of I2C interfaces 
// on the Pico, the handler can delegate the interrupt to the correct instance based on 
// the originating I2C interface.
picoSlaveI2C *ContextI2C0 = nullptr, *ContextI2C1 = nullptr;

static inline picoSlaveI2C* getContext(i2c_inst_t *i2c) {
	return  (i2c == nullptr) ? (nullptr) : ( (i2c == i2c0) ? ContextI2C0 : ContextI2C1 );
}

// I2C interface interrupt handler, that calls picoSlaveI2C object methods based on I2C events.
void I2CInterruptHandler(i2c_inst_t *i2c, i2c_slave_event_t event) {
	picoSlaveI2C *context = getContext(i2c);
	
	switch (event) {
	
	// Master has written some data
	case I2C_SLAVE_RECEIVE:
		context->writeHandler(i2c_read_byte_raw(i2c));
		break;
	
	// Master is requesting data
	case I2C_SLAVE_REQUEST:
		i2c_write_byte_raw(i2c, context->readHandler());
		break;

	default:
		break;
	}
}

picoSlaveI2C::picoSlaveI2C():
	i2cInstance(nullptr)
{}

picoSlaveI2C::~picoSlaveI2C() {
	picoSlaveI2C *toClear = getContext(this->i2cInstance);

	if (toClear != nullptr) {
		toClear = nullptr;
	}
}

void picoSlaveI2C::initialize(uint8_t scl, uint8_t sda, i2c_inst_t *i2c, uint32_t i2cFreqKHz, uint8_t i2c_address, uint8_t *memory, uint32_t memorySize) {
	// Initialize SDA pin
	gpio_init(sda);
	gpio_set_function(sda, GPIO_FUNC_I2C);
	gpio_pull_up(sda);

	// Initialize SCL pin
	gpio_init(scl);
	gpio_set_function(scl, GPIO_FUNC_I2C);
	gpio_pull_up(scl);

	// Initialize i2c
	i2c_init(i2c, i2cFreqKHz * 1000);
	i2c_slave_init(i2c, i2c_address, &I2CInterruptHandler);

	// Set this object as context in I2C interface used by this object. 
	if (i2c == i2c0) {
		ContextI2C0 = this;
	} else {
		ContextI2C1 = this;
	}

	GenericSlave::initialize(memory, memorySize);
}
