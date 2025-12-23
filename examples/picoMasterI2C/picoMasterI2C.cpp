/*
picoMasterI2C.cpp

Example usage of picoMasterI2C class
Uses pico's i2c1 port with pin 14 as sda and pin 15 as scl.
Requaries slave connected by I2C bus with at least 18 byte memory and 17 byte receive buffer.

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
#include <pico/stdlib.h>
#include <hardware/i2c.h>

#include <picoMasterI2C.hpp>

constexpr i2c_inst_t* I2C =  i2c1;
constexpr uint8_t SDA = 14;
constexpr uint8_t SCL = 15;
constexpr uint32_t I2C_FREQ_KHz = 1000;
uint8_t SLAVE_I2C_ADDRESS = 0x17; 

int main() {
	stdio_init_all();
	sleep_ms(4000);
	printf("Pico I2C master example\n");
	
	picoMasterI2C master(SCL, SDA, I2C, I2C_FREQ_KHz);

	uint8_t buffer[128] = {}; // Buffer for writing/reading data
	StatusValue status;
	
	while (true) {
		// Increment each value in buffer.
		for (uint32_t i = 0; i < 16; i++) {
			buffer[i]++;
		}

		status = master.write(SLAVE_I2C_ADDRESS, 0, buffer, 16, true);
		printf("Write status: %02x\n", status);

		while ((status = master.readStatus(SLAVE_I2C_ADDRESS)) & Busy) {
		 	printf("waiting for Ok(0x80) status, received status =  %02x\n", status);
		 	sleep_ms(100);
		}

		uint8_t rbuffer[16];

		status = master.read(SLAVE_I2C_ADDRESS, 0, rbuffer, 16);
		printf("Read status: %02x\n Received data: ", status);
		
		for (uint8_t i = 0; i < 16; i++) {
			printf("%02x ", rbuffer[i]);
		}
		printf("\n");

		sleep_ms(1000);
	}
}
