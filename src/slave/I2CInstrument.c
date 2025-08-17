#include "I2CInstrument.h"

struct i2c_context {
    uint8_t *memory;
    uint32_t memory_size;
	uint32_t memory_address;
    uint32_t transfer_size;
	uint32_t bytes_received;
    uint8_t address_size;
    uint8_t address_received : 1;
    uint8_t size_received : 1;
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

        // Set address received flag if all address bytes were collected.
        if ( (!context->address_received) && (context->bytes_received == context->address_size) ) {
            context->address_received = true;
            context->bytes_received = 0;
        }

        // Set size_received flag if all size bytes were collected (address already received).
        if ( (context->address_received) && (!context->size_received) && (context->bytes_received == context->address_size) ) {
            context->size_received = true;
            context->bytes_received = 0;
        }

        if (!context->address_received) {
			// Receive memory address (can be multiple bytes long)
            context->memory_address |= (uint32_t)i2c_read_byte_raw(i2c) << (context->bytes_received * 8 );
            context->bytes_received++;
        } else if (!context->size_received) {
            context->transfer_size |= (uint32_t)i2c_read_byte_raw(i2c) << (context->bytes_received * 8 );
            context->bytes_received++;
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
        // Reset all values
        context->memory_address = 0;
		context->bytes_received = 0;
        context->transfer_size = 0;
        context->address_received = false;
        context->size_received = false;
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
	context->bytes_received = 0;
	context->memory_address = 0;
    context->transfer_size = 0;
    context->address_received = false;
    context->size_received = false;
}
