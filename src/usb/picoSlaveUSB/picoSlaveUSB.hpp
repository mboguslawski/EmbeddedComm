/*
picoSlaveUSB.h

USB slave class dedicated for raspberry pi pico (rp2040, rp2350).

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

#include "pico/stdlib.h"
#include "tusb.h"
#include "bsp/board_api.h"
#include "usb_descriptors.h"
#include "device/usbd.h"
#include "GenericSlave.hpp"

// Only one instance of this class allowed
class picoSlaveUSB : public GenericSlave {
public:
	picoSlaveUSB();
	~picoSlaveUSB();

	// Initialize usb slave with allocated memory.
	void initialize(uint8_t *memory, uint32_t memorySize);

	// Needs to be called frequentlly (eg. in main loop).
	void process();

	// Handles data received from master.
	void bulkOutHandler(uint8_t itf, uint8_t const* buffer, uint16_t bufsize);

protected:
	// Invoked by parent class. Modify bytesToSend value.
	void sendToMaster(uint32_t nBytes) override;

private:
	// If master requested read, send data to tx buffer.
	void bulkInHandler();

	uint32_t bytesToSend;
};

extern picoSlaveUSB USBSlave;
