/*
USB device descriptor information

Copyright (C) 2025 Mateusz Bogusławski, E: mateusz.boguslawski@ibnet.pl

Based on https://github.com/piersfinlayson/tinyusb-vendor-example
Copyright (c) 2025 Piers Finlayson <piers@piers.rocks>
*/

#ifndef USB_DEVICE_VID
#define USB_DEVICE_VID 0x1111
#endif

#ifndef USB_DEVICE_PID
#define USB_DEVICE_PID 0x1111
#endif

// Maximum packet sizes.
// Pico supports only full-speed device, which limits maximum packet size to 64
#define MAX_ENDPOINT0_SIZE  64
#define ENDPOINT_BULK_SIZE  64

// Manufacturer name used in usb descriptor.
#ifndef USB_MANUFACTURER
#define USB_MANUFACTURER  "mboguslawski"
#endif

// Product name used in usb descriptor.
#ifndef USB_PRODUCT
#define USB_PRODUCT "EmmbeddedComm USB example"
#endif

// Serial used in usb descriptor.
#ifndef USB_SERIAL
#define USB_SERIAL "000"
#endif

// Indexes for the strings in the USB device descriptor
enum {
    STRID_LANGID = 0,
    STRID_MANUFACTURER,
    STRID_PRODUCT,
    STRID_SERIAL,
};

// Interfaces for the USB device descriptor
enum {
    ITF_NUM_VENDOR = 0,
    ITF_NUM_TOTAL
};

// Bulk transfer endpoints numbers
#define BULK_IN_ENDPOINT_DIR   0x81
#define BULK_OUT_ENDPOINT_DIR  0x01
