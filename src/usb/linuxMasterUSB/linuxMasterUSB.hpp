/*
linuxMasterUSB.hpp

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

#include "../../GenericMaster.hpp"

#include <libusb-1.0/libusb.h>
#include <cstdlib>
#include <map>

// Product ID and vendor ID pair can identify usb slave device.
struct slaveInfo {
	uint16_t PID; // Product ID
	uint16_t VID; // Vendor ID

	bool operator==(const slaveInfo& other) const {
        return VID == other.VID && PID == other.PID;
    }

	bool operator<(const slaveInfo& other) const {
        return VID < other.VID && PID < other.PID;
    }
};

class linuxMasterUSB : public GenericMaster<slaveInfo> {
public:
	linuxMasterUSB();
	~linuxMasterUSB();

protected:
	int readBytes(slaveInfo &slave, uint8_t *byteArray, uint32_t numberOfBytes) override;
	int writeBytes(slaveInfo &slave, uint8_t *byteArray, uint32_t numberOfBytes) override;

private:

	libusb_device_handle* openDevice(slaveInfo &slave);

	std::map<slaveInfo, libusb_device_handle*> openedDevices;
	libusb_context* ctx;
};

