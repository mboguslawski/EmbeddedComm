#include "I2CInstrument.h"

struct i2c_context {
    uint8_t *memory;
    uint32_t memory_size;
	uint8_t address_size;
	uint32_t memory_address;
	uint8_t address_bytes_received;
};

static volatile struct i2c_context i2c0_context, i2c1_context;

static inline volatile struct i2c_context *get_context(i2c_inst_t *i2c) {
    return (i2c == i2c0) ? &i2c0_context : &i2c1_context;
}

static void i2c_instrument_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
	volatile struct i2c_context *context = get_context(i2c);
	
	switch (event) {
	
	// Master has written some data
    case I2C_SLAVE_RECEIVE:
		// Address bytes are transmitted first
        if (context->address_bytes_received < context->address_size) {
			// Receive memory address (can be multiple bytes long)
            context->memory_address |= (uint32_t)i2c_read_byte_raw(i2c) << (context->address_bytes_received * 8 );
            context->address_bytes_received++;
        } else {
            // Save into memory
            context->memory[context->memory_address] = i2c_read_byte_raw(i2c);
            context->memory_address++;
        }
        break;
	
	// Master is requesting data
    case I2C_SLAVE_REQUEST:
        // Load from memory
        i2c_write_byte_raw(i2c, context->memory[context->memory_address]);
        context->memory_address++;
        break;
	
	// Master has signalled Stop or Restart
	case I2C_SLAVE_FINISH:
        context->memory_address = 0;
		context->address_bytes_received = 0;
        break;
    
	default:
        break;
    }
}

inline uint8_t bytes_needed(uint64_t n) {
    uint8_t result = 1; // at least 1 byte
    result += (n > 0xFF); // needs >1 byte
    result += (n > 0xFFFF); // needs >2 bytes
    result += (n > 0xFFFFFF); // needs >3 bytes
    result += (n > 0xFFFFFFFF); // needs >4 bytes
    result += (n > 0xFFFFFFFFFFULL); // needs >5 bytes
    result += (n > 0xFFFFFFFFFFFFULL); // needs >6 bytes
    result += (n > 0xFFFFFFFFFFFFFFULL); // needs >7 bytes
    return result;
}

void i2c_instrument_init(uint8_t scl, uint8_t sda, i2c_inst_t *i2c, uint8_t i2c_address, uint8_t *memory, uint32_t memory_size) {
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
    i2c_slave_init(i2c, i2c_address, &i2c_instrument_handler);

	// Assign values to context that corresponds to i2c instance
	volatile struct i2c_context* context = get_context(i2c);
	context->memory = memory;
	context->memory_size = memory_size;
	context->address_size = bytes_needed(memory_size);
	context->address_bytes_received = 0;
	context->memory_address = 0;
}
