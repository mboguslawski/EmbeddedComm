#include "linuxMasterUSB.hpp"

linuxMasterUSB::linuxMasterUSB():
	ctx(nullptr)
{
	libusb_init(&ctx);
}

linuxMasterUSB::~linuxMasterUSB() {
	// Close and release all devices.
	for (const auto &dev : openedDevices) {
		libusb_release_interface(dev.second, 0);
    	libusb_close(dev.second);
	}
}

int linuxMasterUSB::writeBytes(slaveInfo &slave, uint8_t *byteArray, uint32_t numberOfBytes) {
	libusb_device_handle* dev = openDevice(slave);
	if (dev == nullptr) {
		return 0;
	}

	int written = 0;
	int ret = 0;
	ret = libusb_bulk_transfer(dev, 0x01, byteArray, numberOfBytes, &written, 1000000);

	if (ret < 0) {
		return ret;
	}

	return written;
}


int linuxMasterUSB::readBytes(slaveInfo &slave, uint8_t *byteArray, uint32_t numberOfBytes) {
	libusb_device_handle* dev = openDevice(slave);
	if (dev == nullptr) {
		return 0;
	}

	// One transfer can have up to 64 bytes. Sometimes single read will be not enough.
	uint32_t toRead = numberOfBytes;
	while (toRead > 0) {
		int bytesRead = 0;
		int ret = libusb_bulk_transfer(dev, 0x81, &byteArray[numberOfBytes-toRead], std::min((uint32_t)64, toRead), &bytesRead, 1000000);
		
		if (ret < 0) {
			return ret;
		}

		toRead -= std::min((uint32_t)64, toRead);
	}


	return numberOfBytes;
}

libusb_device_handle* linuxMasterUSB::openDevice(slaveInfo &slave) {
	if (openedDevices.find(slave) != openedDevices.end()) {
		// Device already opened, ready to use.
		return openedDevices[slave];
	}

	libusb_device_handle *devHandle = libusb_open_device_with_vid_pid(ctx, slave.VID, slave.PID);

	if (devHandle == nullptr) {
		return nullptr;
	}

	// Detach kernel driver if active.
	if (libusb_kernel_driver_active(devHandle, 0)) {
		libusb_detach_kernel_driver(devHandle, 0);
	}

	if (libusb_claim_interface(devHandle, 0) < 0) {
		libusb_close(devHandle);
		return nullptr;
	}

	std::pair<slaveInfo, libusb_device_handle*> toInsert;
	toInsert.first = slave;
	toInsert.second = devHandle;

	openedDevices.insert(toInsert);
	
	return devHandle;
}
