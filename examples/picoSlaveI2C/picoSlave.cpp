/*
picoSlave.cpp

picoSlaveI2C class example usage

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

#include <pico/stdlib.h>
#include <stdio.h>
#include <string.h>

#include "picoSlaveI2C.hpp"

static const uint I2C_SLAVE_ADDRESS = 0x17;
static const uint SDA = 16; // 4
static const uint SCL = 17; // 5
static const uint32_t i2cFrequencyKHz = 1000; // 1MHz
static const uint32_t SET_LED_ADDRESS = 0x0;
static const uint32_t COUNTER_ADDRESS = 0x1; // Four bytes
uint8_t memory[64];
uint8_t buffer[16];


void setLed();

int main() {
	stdio_init_all();
	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
	
	memory[COUNTER_ADDRESS] = 0;
	memory[COUNTER_ADDRESS+1] = 0;
	memory[COUNTER_ADDRESS+2] = 0;
	memory[COUNTER_ADDRESS+3] = 0;

	picoSlaveI2C slave;

	slave.initialize(SCL, SDA, i2c0, i2cFrequencyKHz, I2C_SLAVE_ADDRESS, memory, 64);
	slave.enableMemBackups(buffer, 16);
	slave.addMemoryChangeCallback(0, setLed);

	while (true) {
	  	slave.process();
	}
}

void setLed() {
	gpio_put(PICO_DEFAULT_LED_PIN, memory[SET_LED_ADDRESS]);
	uint32_t *counter = (uint32_t*)(memory + COUNTER_ADDRESS);
	
	(*counter)++;
}
