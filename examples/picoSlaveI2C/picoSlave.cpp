#include <pico/stdlib.h>
#include <stdio.h>
#include <string.h>

#include "picoSlaveI2C.hpp"

static const uint I2C_SLAVE_ADDRESS = 0x17;
static const uint SDA = 16;
static const uint SCL = 17;

uint8_t memory[256];
uint8_t buffer[64];

int main() {
	stdio_init_all();

	initPicoSlaveI2C(SCL, SDA, i2c0, I2C_SLAVE_ADDRESS, memory, 256, buffer, 64);

	while (true) {
		tight_loop_contents();
	}
}
