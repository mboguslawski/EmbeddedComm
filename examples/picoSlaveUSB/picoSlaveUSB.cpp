/*
picoSlaveUSB.cpp

picoSlaveUSB class example usage

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

#include "picoSlaveUSB.hpp"

static const uint32_t SET_LED_ADDRESS = 2000;
static const uint32_t COUNTER_ADDRESS = 2001; // Four bytes
uint8_t memory[2048];
uint8_t buffer[16];

extern void foo();
void setLed();

int main() {
	stdio_init_all();
	sleep_ms(4000);
	printf("picoSlaveUSB example\n");
	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
	
	memory[COUNTER_ADDRESS] = 0;
	memory[COUNTER_ADDRESS+1] = 0;
	memory[COUNTER_ADDRESS+2] = 0;
	memory[COUNTER_ADDRESS+3] = 0;

	USBSlave.initialize(memory, 2048);
	USBSlave.enableMemBackups(buffer, 16);
	USBSlave.addMemoryChangeCallback(2000, setLed);

	while (true) {
	  	USBSlave.process();
	}
}

void setLed() {
	gpio_put(PICO_DEFAULT_LED_PIN, memory[SET_LED_ADDRESS]);
	uint32_t *counter = (uint32_t*)(memory + COUNTER_ADDRESS);
	
	(*counter)++;
}
