#ifndef I2C_ORCHESTRA_ERRORS
#define I2C_ORCHESTRA_ERRORS

#define I2C_O_OK 					0x00 // No errors
#define I2C_O_ERR_INVALID_ADDR 		0x01 // Memory address is not valid, falls outside memory range.
#define I2C_O_ERR_SIZE_MISMATCH 	0x02 // Number of bytes master is sending/requesting not same as declared in transfer_sizes.
#define I2C_O_ERR_WBUFFER_OVERFLOW 	0x04 // Slave's write buffer (if enabled) is overflowing, reduce packet size or make slave's write buffer bigger.
#define I2C_O_ERR_DATA_CORRUPTED 	0x08 // Calculated checksum does not match the received one
#define I2C_O_ERR_INVALID_FLOW 		0x10 // Master is requesting data when he should send or sending data when he is supposed to request. 

typedef uint8_t status_register_t;

#endif
