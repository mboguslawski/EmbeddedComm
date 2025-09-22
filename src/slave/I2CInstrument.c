#include "I2CInstrument.h"

struct i2c_context {
    uint8_t *memory;
    uint8_t *write_buffer;
    volatile uint32_t max_transfer_size;
    volatile uint32_t memory_size;
	volatile uint32_t memory_address;
    volatile uint32_t transfer_size;
	volatile uint32_t byte_counter;
    volatile uint8_t checksum;
    volatile uint8_t address_size;
    volatile uint8_t transfer_state;
    volatile status_register_t status_register;
};

// Possible transfer states
#define RECEIVING_ADDRESS 0x00
#define RECEIVING_TSIZE 0x01
#define RECEIVING_DATA 0x02
#define RECEIVING_CHECKSUM 0x03
#define SENDING_DATA 0x04
#define SENDING_STATUS_REG 0x05
#define SENDING_CHECKSUM 0x06
#define TRANSFER_FINISHED 0x07

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

    // Move to the next state if all address bytes were collected.
    if (context->byte_counter == context->address_size) {
        context->transfer_state = RECEIVING_TSIZE;
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
    
    // Move to the next state if all size bytes were collected (address already received).
    if (context->byte_counter == context->address_size) {
        context->transfer_state = RECEIVING_DATA;
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
        context->transfer_state = RECEIVING_CHECKSUM;
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
    context->transfer_state = RECEIVING_ADDRESS;
    context->checksum = 0;
}

// Compare received checksum with calculated and set error flag in status register if thoose do not match.
static inline void check_checksum(struct i2c_context *context, uint8_t checksum_byte) {
    if (context->checksum != checksum_byte) {
        context->status_register |= I2C_O_ERR_DATA_CORRUPTED;
    }

    context->transfer_state = TRANSFER_FINISHED;
}

// Handle all logic related to slave receiving byte from master.
static inline void write_handler(struct i2c_context *context, uint8_t received_byte) {
    // Reset context if this is new transfer
    if (context->transfer_state == TRANSFER_FINISHED) {
        reset_context(context);
    }
    
    // Discard byte if any error was detected
    if (context->status_register != 0) {
        return;
    }

    // Include byte in checksum calculation (except received checksum byte).
    if (context->transfer_state != RECEIVING_CHECKSUM) {
        context->checksum = calc_checksum_it(context->checksum, received_byte);
    }

    // Decide where to save receivied byte according to current state.
    switch (context->transfer_state) {
    
    case RECEIVING_ADDRESS:
        write_memory_address(context, received_byte);
        break;

    case RECEIVING_TSIZE:
        write_transfer_size(context, received_byte);
        break;
    
    case RECEIVING_DATA:
        write_memory(context, received_byte);
        break;

    case RECEIVING_CHECKSUM:
        check_checksum(context, received_byte);
        break;
        
    case TRANSFER_FINISHED:
        context->status_register |= I2C_O_ERR_SIZE_MISMATCH;
        break;

    default:
        context->status_register |= I2C_O_ERR_INVALID_FLOW;
        break;
    }
}

static inline uint8_t read_handler(struct i2c_context *context) {
    uint8_t out_byte = 0x0;

    // If read address not received, return status register
    if (context->transfer_state == TRANSFER_FINISHED) {

        context->transfer_state = SENDING_STATUS_REG;        
        out_byte = context->status_register;
        context->status_register = 0x0;

        context->transfer_state = TRANSFER_FINISHED;

        return out_byte;
    }

    if (context->transfer_state == RECEIVING_DATA || context->transfer_state == SENDING_DATA) {
        context->transfer_state = SENDING_DATA;

        // Return byte from memory, increamenting byte counter, so the following byte will be returned next.
        out_byte = context->memory[context->memory_address + context->byte_counter];
        context->checksum = calc_checksum_it(context->checksum, out_byte);
        context->byte_counter++;

        if (context->byte_counter == context->transfer_size) {
            context->transfer_state = SENDING_CHECKSUM;
        }
        
        return out_byte;
    }

    if (context->transfer_state == SENDING_CHECKSUM) {
        // Send checksum byte after all data has been send out
        out_byte = context->checksum;
        context->transfer_state = TRANSFER_FINISHED;
    }

    return out_byte;
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
        i2c_write_byte_raw(i2c, read_handler(context));
        break;
	
	// Master has signalled Stop or Restart
	case I2C_SLAVE_FINISH:

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
    context->transfer_state = TRANSFER_FINISHED;
    context->status_register = 0;
    context->checksum = 0;
}
