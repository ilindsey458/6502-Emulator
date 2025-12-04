#include <stdint.h>

typedef uint8_t byte;
typedef uint16_t word;

static byte memory[0x10000];

// Set all elements of array to 0
void init_memory() {
	// memset(memory, 0, sizeof(memory));
	for (int i = 0; i < sizeof(memory); i++) { memory[i] = 0; }
}

// Returns byte at address in memory
byte read_6502(word address) {
	return memory[address];
}

// Writes byte to address in memory
void write_6502(word address, byte p_value) {
	memory[address] = p_value;
}

// Loads byte array into memory
void load_program(word start_address, const byte *program, word length) {
	for (word i = 0; i < length; i++) {
		memory[start_address + i] = program[i];
	}
}
