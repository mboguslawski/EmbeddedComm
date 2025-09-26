/*
picoSlaveI2C.cpp

Example usage of picoSlaveI2C class
Uses pico's i2c0 port with pin 16 as sda and pin 17 as scl.

Copyright (C) 2025 Mateusz Bogusławski, E: mateusz.boguslawski@ibnet.pl

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see https://www.gnu.org/licenses/.
*/

#include <pico/stdlib.h>
#include <stdio.h>
#include <string.h>

#include "picoSlaveI2C.hpp"

static const uint I2C_SLAVE_ADDRESS = 0x17;
static const uint SDA = 16; // 4
static const uint SCL = 17; // 5
static const uint32_t i2cFrequencyKHz = 1000; // 1MHz

uint8_t memory[256];
uint8_t buffer[64];

int main() {
    stdio_init_all();

    picoSlaveI2C slave;

    slave.initialize(SCL, SDA, i2c0, i2cFrequencyKHz, I2C_SLAVE_ADDRESS, memory, 256, buffer, 64);

    while (true) {
      tight_loop_contents();
    }
}
