#include "I2CInstrument.h"

struct i2c_context {
    uint8_t *memory;
    uint8_t *write_buffer;
    volatile uint32_t max_transfer_size;
    volatile uint32_t memory_size;
	volatile uint32_t memory_address;
    volatile uint32_t transfer_size;
	volatile uint32_t byte_counter;
    volatile uint8_t address_size;
    volatile uint8_t transfer_state;
    volatile uint8_t reset_pending;
    volatile status_register_t status_register;
};

// Possible transfer states
#define NEW_TRANSFER 0x0
#define ADDRESS_RECEIVED 0x1
#define SIZE_RECEIVED 0x2
#define DATA_RECEIVED 0x3

static struct i2c_context i2c0_context, i2c1_context;

static inline struct i2c_context *get_context(i2c_inst_t *i2c) {
    return (i2c == i2c0) ? &i2c0_context : &i2c1_context;
}

// Writes byte to context->memory_address.
// Sets context->address_received flag, resets context->byte_counter 
// and sets error flag in status register if all address bytes received.
static inline void write_memory_address(struct i2c_context *context, uint8_t byte) {
    // Receive memory address (can be multiple bytes long)
    context->memory_address |= (uint32_t)byte << (context->byte_counter * 8 );
    context->byte_counter++;

    // Set address received flag if all address bytes were collected.
    if (context->byte_counter == context->address_size) {
        context->transfer_state = ADDRESS_RECEIVED;
        context->byte_counter = 0;

        // Check address
        if (context->memory_address >= context->memory_size) {
            context->status_register |= I2C_O_ERR_INVALID_ADDR;
        }
    }
}

// Writes byte to context->transfer_size.
// Sets context->size_received flag, resets context->byte_counter 
// and sets error flag in status register if all transfer size bytes received.
static inline void write_transfer_size(struct i2c_context *context, uint8_t byte) {
    // Receive transfer size (can be multiple bytes long)
    context->transfer_size |= (uint32_t)byte << (context->byte_counter * 8 );
    context->byte_counter++;
    
    // Set size_received flag if all size bytes were collected (address already received).
    if (context->byte_counter == context->address_size) {
        context->transfer_state = SIZE_RECEIVED;
        context->byte_counter = 0;

        // Check if declared transfer size does not exceed write_buffer size (if enabled)
        if ( (context->write_buffer != NULL) && (context->transfer_size > context->max_transfer_size) ) {
            context->status_register |= I2C_O_ERR_WBUFFER_OVERFLOW;
        }

        // Check if data will not overflow memory
        if (context->memory_address + context->transfer_size >= context->memory_size) {
            context->status_register |= I2C_O_ERR_INVALID_ADDR;
        }
    }
}

// Writes received byte to memory/write buffer and checks for errors.
static inline void write_memory(struct i2c_context *context, uint8_t byte) {
    // Save received byte
    if (context->write_buffer != NULL) {
        // To write buffer if enabled
        context->write_buffer[context->byte_counter] = byte;
    } else {
        // Directly to memory, if write buffer disabled
        context->memory[context->memory_address + context->byte_counter] = byte;
    }

    context->byte_counter++;

    if (context->byte_counter == context->transfer_size) {
        context->transfer_state = DATA_RECEIVED;
    }
}

// Resets context after i2c transfer. Needs to be done before new transfer handling.
static inline void reset_context(struct i2c_context *context) {
    // Copy data from write_buffer to memory
    if ( (context->write_buffer != NULL) && (context->status_register == 0) ) {
        uint32_t size = MIN(context->max_transfer_size, context->transfer_size);
        memcpy(context->memory + context->memory_address, context->write_buffer, size);
    }

    // Reset all values
    context->memory_address = 0;
	context->byte_counter = 0;
    context->transfer_size = 0;
    context->transfer_state = NEW_TRANSFER;
    context->reset_pending = false;
}

// Handle all logic related to slave receiving byte from master.
static inline void write_handler(struct i2c_context *context, uint8_t received_byte) {
    // Reset context if this is new transfer
    if (context->reset_pending) {
        reset_context(context);
    }
    
    // Discard byte if any error was detected
    if (context->status_register != 0) {
        return;
    }

    // Decide where to save receivied byte according to current state.
    switch (context->transfer_state) {
    
    case NEW_TRANSFER:
        write_memory_address(context, received_byte);
        break;

    case ADDRESS_RECEIVED:
        write_transfer_size(context, received_byte);
        break;
    
    case SIZE_RECEIVED:
        write_memory(context, received_byte);
        break;

    case DATA_RECEIVED:
    default:
        context->status_register |= I2C_O_ERR_SIZE_MISMATCH;
        break;
    }
}

// Handles all i2c communication logic on slave side.
// Is executed as interrupt routine.
static void i2c_instrument_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
	struct i2c_context *context = get_context(i2c);
	
	switch (event) {
	
	// Master has written some data
    case I2C_SLAVE_RECEIVE:
        uint8_t in_byte = i2c_read_byte_raw(i2c);
        write_handler(context, in_byte);
        break;
	
	// Master is requesting data
    case I2C_SLAVE_REQUEST:
        i2c_write_byte_raw(i2c, 0x0);
        break;
	
	// Master has signalled Stop or Restart
	case I2C_SLAVE_FINISH:
        context->reset_pending = true;
        break;
	default:
        break;
    }
}

void i2c_instrument_enable_write_buffer(i2c_inst_t *i2c, uint8_t *memory, uint32_t memory_size) {
    volatile struct i2c_context* context = get_context(i2c);

    context->write_buffer = memory;
    context->max_transfer_size = memory_size;
}

status_register_t i2c_instrument_get_status(i2c_inst_t *i2c) {
    return get_context(i2c)->status_register;
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
	struct i2c_context* context = get_context(i2c);
	context->memory = memory;
	context->memory_size = memory_size;
    context->write_buffer = NULL;
    context->max_transfer_size = 0;
	context->address_size = bytes_needed(memory_size);
	context->byte_counter = 0;
	context->memory_address = 0;
    context->transfer_size = 0;
    context->transfer_state = NEW_TRANSFER;
    context->status_register = 0;
    context->reset_pending = false;
}
