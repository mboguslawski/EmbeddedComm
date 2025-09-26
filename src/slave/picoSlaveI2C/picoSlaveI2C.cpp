#include "picoSlaveI2C.hpp"

GenericSlave ContextI2C0, ContextI2C1;

static inline GenericSlave* getContext(i2c_inst_t *i2c) {
	return (i2c == i2c0) ? &ContextI2C0 : &ContextI2C1;
}

void I2CInterruptHandler(i2c_inst_t *i2c, i2c_slave_event_t event) {
	GenericSlave *context = getContext(i2c);
	
	switch (event) {
	
	// Master has written some data
	case I2C_SLAVE_RECEIVE:
		context->writeHandler(i2c_read_byte_raw(i2c));
		break;
	
	// Master is requesting data
	case I2C_SLAVE_REQUEST:
		i2c_write_byte_raw(i2c, context->readHandler());
		break;
	
	// Master has signalled Stop or Restart
	case I2C_SLAVE_FINISH:
		context->stopHandler();
		break;

	default:
		break;
	}
}

void initPicoSlaveI2C(uint8_t scl, uint8_t sda, i2c_inst_t *i2c, uint8_t i2c_address, uint8_t *memory, uint32_t memorySize, uint8_t *rBuffer, uint32_t rBufferSize) {
	// Initialize SDA pin
	gpio_init(sda);
	gpio_set_function(sda, GPIO_FUNC_I2C);
	gpio_pull_up(sda);

	// Initialize SCL pin
	gpio_init(scl);
	gpio_set_function(scl, GPIO_FUNC_I2C);
	gpio_pull_up(scl);

	// Initialize i2c with 1MHz frequency
	i2c_init(i2c, 1*1000*1000);
	i2c_slave_init(i2c, i2c_address, &I2CInterruptHandler);

	getContext(i2c)->initialize(memory, memorySize, rBuffer, rBufferSize);
}
