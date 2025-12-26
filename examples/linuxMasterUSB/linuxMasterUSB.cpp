/*
linuxMasterUSB.cpp

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

#include <iostream>
#include <unistd.h>
#include "./lib/EmbeddedComm/src/usb/linuxMasterUSB/linuxMasterUSB.hpp"

int main() {
	printf("Linux USB master example\n");

	linuxMasterUSB master;
	slaveInfo slave;
	slave.PID = 0x1111;
	slave.VID = 0x1111;
	
	StatusValue status;
	
	uint8_t led = 0;
	uint8_t dataBuffer[5];

	while (true) {
		// Toggle state
		led = (led == 0);

		// Set slave's led state
		status = master.write(slave, 2000, &led, 1);
		printf("Write status: %02xh\n", status);

		while ((status = master.readStatus(slave)) & Busy) {
		 	printf("waiting for Ok(0x80) status, received status =  %02xh\n", status);
		 	usleep(500);
		}


		// Read slave's led state and counter.
		status = master.read(slave, 2000, dataBuffer, 5);
		printf("Read status: %02xh\n", status);
		
		printf("Slave's current led state: %u\n", dataBuffer[0]);
		printf("Slave's current counter value: %u\n", *(uint32_t*)(dataBuffer+1));

		sleep(1);
	}
}
