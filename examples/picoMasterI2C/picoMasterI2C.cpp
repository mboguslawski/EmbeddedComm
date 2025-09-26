/*
picoMasterI2C.cpp

Example usage of picoMasterI2C class
Uses pico's i2c1 port with pin 14 as sda and pin 15 as scl.

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

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <picoMasterI2C.hpp>

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define I2C_FREQ_KHz 1000 // 1Mhz

int main() {
	stdio_init_all();
	sleep_ms(4000);
		
	puts("Pico I2C master example\n");
	
	picoMasterI2C master(I2C_SCL, I2C_SDA, I2C_PORT, I2C_FREQ_KHz);
	uint8_t slaveAddress = 0x17;

	uint8_t buffer[16] = {};
	uint8_t status;
	uint8_t slave_tchecksum;
	
	int i = 0;
	while (true) {
		printf("%d\n", i);
		i++;
		
		for (uint32_t i = 0; i < 16; i++) {
			buffer[i] += 1;
		}

		master.write(slaveAddress, 255-16, buffer, 16);
		master.readStatus(slaveAddress, &status);
		printf("Write status %u\n", status);


		for (uint32_t i = 0; i < 16; i++) {
			buffer[i] = 0;
		}

		master.read(slaveAddress, 255-16, buffer, 16);
		master.readTChecksum(slaveAddress, &slave_tchecksum);
		master.readStatus(slaveAddress, &status);
		printf("Read status %u\n", status);

		printf("Data:\n");
		for (uint32_t i = 0; i < 16; i++) {
			printf("%u %u\n", i, buffer[i]);
		}

		sleep_ms(1000);
	}
}
