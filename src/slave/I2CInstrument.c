#include "I2CInstrument.h"

// Possible transfer states
typedef enum {
	STATE_IDLE,             // Idle, ready for new transfer
	STATE_R_ADDRESS,        // Receiving memory address
	STATE_R_DATA,           // Receiving data bytes
	STATE_T_DATA,           // Transmitting data
} transfer_state_t;

static const uint32_t ADDRESS_SIZE = 4; // Bytes

struct i2c_context {
	uint8_t *memory;
	uint8_t *write_buffer;
	uint8_t *status_byte;
	uint32_t max_transfer_size;
	uint32_t memory_size;
	volatile uint32_t memory_address;
	volatile uint32_t byte_counter;
	volatile transfer_state_t transfer_state;
};

static struct i2c_context i2c0_context, i2c1_context;

static inline struct i2c_context *get_context(i2c_inst_t *i2c) {
	return (i2c == i2c0) ? &i2c0_context : &i2c1_context;
}

// Move received data from master to memory.
static inline void write_memory(struct i2c_context *context) {

	// Copy data from write_buffer to memory
	if ( (context->transfer_state == STATE_R_DATA) && ((*context->status_byte) == I2C_O_OK) ) {
		memcpy(context->memory + context->memory_address, context->write_buffer, context->byte_counter);
	}
}

// Set given error flag in status register
static inline void set_error_flag(struct i2c_context *context, uint8_t error) {
	(*context->status_byte) |= error;
} 

static inline void change_transfer_state(struct i2c_context *context, transfer_state_t new_state) {
	switch (new_state) {
	
	case STATE_IDLE:
		write_memory(context);
		break;

	case STATE_R_ADDRESS:
		context->memory_address = 0;
		context->byte_counter = 0;
		break;
	
	case STATE_R_DATA:
		context->byte_counter = 0;
		break;

	case STATE_T_DATA:
		context->byte_counter = 0;
		break;
	}

	context->transfer_state = new_state;
}

// Copies first four bytes from wbuffer to context->memory_address
// and sets error is address is not valid.
static inline void write_memory_address(struct i2c_context *context) {
	// Receive memory address (can be multiple bytes long)
	uint32_t new_address = *((uint32_t*)context->write_buffer);

	if (new_address >= context->memory_size) {
		set_error_flag(context, I2C_O_ERR_MEM_OUT_OF_RANGE);
		return;
	}

	context->memory_address = new_address;
}

// Writes received byte to memory/write buffer and checks for errors.
static inline void write_buffer(struct i2c_context *context, uint8_t byte) {
	
	// Check for overflow in write_buffer and/or write outside memory. 
	if ( (context->byte_counter >= context->max_transfer_size) || (context->memory_address + context->byte_counter >= context->memory_size) ) {
		set_error_flag(context, I2C_O_ERR_MEM_OUT_OF_RANGE);
		return;
	}

	context->write_buffer[context->byte_counter] = byte;
	context->byte_counter++;
}

// Handle all logic related to slave receiving byte from master.
static inline void write_handler(struct i2c_context *context, uint8_t received_byte) {
	// Decide where to save receivied byte according to current state.
	if (context->transfer_state == STATE_IDLE) {
		change_transfer_state(context, STATE_R_ADDRESS);
	}

	write_buffer(context, received_byte);

	// Check if memory address is fully received in buffer
	if ( (context->transfer_state == STATE_R_ADDRESS) && (context->byte_counter == ADDRESS_SIZE)) {
		
		// Copy and check address from buffer
		write_memory_address(context);
		change_transfer_state(context, STATE_R_DATA);
	}
}

static inline uint8_t read_handler(struct i2c_context *context) {
	uint8_t out_byte = 0x0;

	if (context->transfer_state == STATE_IDLE) {
		change_transfer_state(context, STATE_T_DATA);
	}
	
	uint32_t idx = context->memory_address + context->byte_counter;

	if (idx >= context->memory_size) {
		set_error_flag(context, I2C_O_ERR_MEM_OUT_OF_RANGE);
		return out_byte;
	}

	out_byte = context->memory[idx];
	context->byte_counter++;

	if (idx == 0) {
		(*context->status_byte) = I2C_O_OK;
	}

	return out_byte;
}

static inline void handle_stop(struct i2c_context *context) {
	
	// Stop signal should not occur in STATE_R_ADDEREE and STATE_IDLE states.
	if ( (context->transfer_state == STATE_R_ADDRESS) || (context->transfer_state == STATE_IDLE) ) {
		set_error_flag(context, I2C_O_ERR_INVALID_ACTION);
	}
	
	// Master couldn't read 0 bytes.
	if ( (context->transfer_state == STATE_T_DATA) && (context->byte_counter == 0) ) {
		set_error_flag(context, I2C_O_ERR_INVALID_ACTION);
	}

	change_transfer_state(context, STATE_IDLE);
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
		handle_stop(context);
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

uint8_t i2c_instrument_get_status(i2c_inst_t *i2c) {
	return (*get_context(i2c)->status_byte);
}

void i2c_instrument_init(uint8_t scl, uint8_t sda, i2c_inst_t *i2c, uint8_t i2c_address, uint8_t *memory, uint32_t memory_size, uint8_t *mem_buffer, uint32_t mem_buffer_size) {
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
	context->byte_counter = 0;
	context->memory_address = 0;
	context->write_buffer = mem_buffer;
	context->max_transfer_size = mem_buffer_size;
	context->status_byte = memory + 0;

	(*context->status_byte) = I2C_O_OK;
	change_transfer_state(context, STATE_IDLE);
}
