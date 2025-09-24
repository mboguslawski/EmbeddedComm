#ifndef I2C_ORCHESTRA_ERRORS
#define I2C_ORCHESTRA_ERRORS

#define I2C_O_OK 					0x00 // No errors
#define I2C_O_ERR_MEM_OUT_OF_RANGE 	0x01 // Memory address is not valid, falls outside memory range.
#define I2C_O_ERR_WBUFFER_OVERFLOW 	0x02 // Slave's write buffer is overflowing, reduce packet size or make slave's write buffer bigger.
#define I2C_O_ERR_INVALID_ACTION	0x04 // Master performed action that is consistent with protocol.

#endif
