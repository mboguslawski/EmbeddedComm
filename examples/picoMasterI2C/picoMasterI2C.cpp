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

	uint8_t buffer[16] = {}; // Buffer for writing/reading data
	uint8_t status;
	uint8_t slave_tchecksum;
	
	int i = 0;
	while (true) {
		printf("%d\n", i);
		i++;
		
		// Increment each value in buffer.
		for (uint32_t i = 0; i < 16; i++) {
			buffer[i] += 1;
		}

		// Write new data to slave and check status
		master.write(SLAVE_I2C_ADDRESS, 2, buffer, 16);
		master.readStatus(SLAVE_I2C_ADDRESS, &status);
		printf("Write status %u\n", status);

		// Erase values in the buffer, to show,
		// that data is read from slave and not remembered by master.
		for (uint32_t i = 0; i < 16; i++) {
			buffer[i] = 0;
		}

		// Read 16 bytes that were previously written to slave's memory;
		master.read(SLAVE_I2C_ADDRESS, 2, buffer, 16);
		master.readTChecksum(SLAVE_I2C_ADDRESS, &slave_tchecksum);
		master.readStatus(SLAVE_I2C_ADDRESS, &status);
		printf("Read status %u\n", status);

		printf("Data:\n");
		for (uint32_t i = 0; i < 16; i++) {
			printf("%u %u\n", i, buffer[i]);
		}

		if (EmbeddedComm::calculateChecksum(buffer, 16) == slave_tchecksum) {
			printf("Checksum matches\n");
		} else {
			printf("Checksum does not match!!!\n");
		}

		sleep_ms(1000);
	}
}
