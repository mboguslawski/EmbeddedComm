#include "picoSlaveUSB.hpp"

picoSlaveUSB USBSlave;

picoSlaveUSB::picoSlaveUSB():
	bytesToSend(0)
{}

picoSlaveUSB::~picoSlaveUSB() {}

void picoSlaveUSB::initialize(uint8_t *memory, uint32_t memorySize) {
	board_init();
    tusb_init();
	
	GenericSlave::initialize(memory, memorySize);
}

void picoSlaveUSB::process() {
	GenericSlave::process();
    bulkInHandler();    
    tud_task();
}

void picoSlaveUSB::bulkOutHandler(uint8_t itf, uint8_t const* buffer, uint16_t bufsize) {
    for (uint16_t i = 0; i < bufsize; i++) {
        writeHandler(buffer[i]);
	}
	
#if CFG_TUD_VENDOR_RX_BUFSIZE > 0
    tud_vendor_read_flush();
#endif
}

void picoSlaveUSB::bulkInHandler() {
    uint32_t txBufferSpace = tud_vendor_write_available();
    if ( (bytesToSend > 0) && (txBufferSpace == ENDPOINT_BULK_SIZE) ) {
        uint8_t trySend = (bytesToSend < txBufferSpace) ? bytesToSend : txBufferSpace;
        uint8_t packet[ENDPOINT_BULK_SIZE] = {0};

        for (uint8_t i = 0; i < trySend; i++) {
            packet[i] = readHandler();
        }

        tud_vendor_write(packet, trySend);
        tud_vendor_write_flush();

        bytesToSend -= trySend;
    }
}

void picoSlaveUSB::sendToMaster(uint32_t nBytes) {
    bytesToSend += nBytes;
}

extern "C" void tud_vendor_rx_cb(uint8_t itf, uint8_t const* buffer, uint16_t bufsize) {
   USBSlave.bulkOutHandler(itf, buffer, bufsize);
}
