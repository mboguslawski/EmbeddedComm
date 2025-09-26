#include "picoMasterI2C.hpp"

picoMasterI2C::picoMasterI2C(uint8_t scl, uint8_t sda, i2c_inst_t *i2c, uint32_t i2cFreqKhz):
	i2cInstance(i2c)
{
	gpio_init(sda);
	gpio_set_function(sda, GPIO_FUNC_I2C);
	gpio_pull_up(sda);

	gpio_init(scl);
	gpio_set_function(scl, GPIO_FUNC_I2C);
	gpio_pull_up(scl);

	i2c_init(i2c, i2cFreqKhz * 1000);
}

int picoMasterI2C::readBytes(uint8_t &slaveAddress, uint8_t *byteArray, uint32_t numberOfBytes) {
	return i2c_read_blocking(i2cInstance, slaveAddress, byteArray, numberOfBytes, false);	
}

int picoMasterI2C::writeBytes(uint8_t &slaveAddress, uint8_t *byteArray, uint32_t numberOfBytes) {
	return i2c_write_blocking(i2cInstance, slaveAddress, byteArray, numberOfBytes, false);	
}
