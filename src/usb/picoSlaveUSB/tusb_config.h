/*
tusb_config.h
TinyUSB configuration for EmbeddedComm Vendor Device
*/

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define BOARD_TUD_RHPORT     	0
#define CFG_TUSB_RHPORT0_MODE   (OPT_MODE_DEVICE|BOARD_TUD_MAX_SPEED)
#define BOARD_TUD_MAX_SPEED  	OPT_MODE_FULL_SPEED
#define CFG_TUD_MAX_SPEED    	BOARD_TUD_MAX_SPEED
#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS           	OPT_OS_NONE
#endif
#define CFG_TUD_ENABLED       	1
#define CFG_TUD_ENDPOINT0_SIZE  64

#define CFG_TUD_VENDOR              1
#define CFG_TUD_VENDOR_EP_BUFSIZE  64
#define CFG_TUD_VENDOR_RX_BUFSIZE  64
#define CFG_TUD_VENDOR_TX_BUFSIZE  64

#define CFG_TUD_CDC     0
#define CFG_TUD_MSC    	0
#define CFG_TUD_HID     0
#define CFG_TUD_MIDI	0
#define CFG_TUD_BTH  	0

#ifdef __cplusplus
}
#endif

#endif