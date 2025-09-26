#pragma once

#define EMBEDDED_C_OK 						0x00 // No errors
#define EMBEDDED_C_ERR_MEM_OUT_OF_RANGE 	0x01 // Memory address is not valid, falls outside memory range.
#define EMBEDDED_C_ERR_WBUFFER_OVERFLOW 	0x02 // Slave's write buffer is overflowing, reduce packet size or make slave's write buffer bigger.
#define EMBEDDED_C_ERR_INVALID_ACTION		0x04 // Master performed action that is consistent with protocol.
#define EMBEDDED_C_ERR_DATA_CORRUPTED		0x08 // Checksum send by master does not match data.
