/*
 * Copyright (c) 2021 Valentin Milea <valentin.milea@gmail.com>
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
