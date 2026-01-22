#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "6502.c"

#define MEM_SIZE 0x10000

static byte memory[MEM_SIZE];

// Set all elements of array to 0
void init_memory() {
	for (int i = 0; i < (sizeof(memory) / sizeof(byte)); i++) { memory[i] = 0; }
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
	instruction_ptr = start_address;
	memory[start_address + length] = 0xDB; 		// Add stop program to end of program
}

static void hexdump() {
	int line_start;
	for (int i = 0; i < sizeof(memory) / (sizeof(byte)); i++) {
		if (memory[i]) {
			line_start = i - (i % 8);
			printf("%X : ", line_start);
			for (int j = 0; j < 7; j++) { printf("%X ", memory[line_start+j]); }
			printf("%X\n", memory[line_start + 7]);
			i += 7;
		}
	}
}


int main(int argc, char **argv) {
	int opt, hexdump_flag = 0;
	char *file_path;	//  HACK: This should use file descriptors and such
	do  { 						//  FIX: This is messy and buggy
		switch(opt = getopt(argc, argv, ":hf:")) {
		case -1:
			file_path = argv[optind];
			break;
		case 'h':
			hexdump_flag = 1;
			break;
		default:
			break;
		}
	} while (opt >= 0);
	byte test_program[30] = {0xA9,0x00,0x85,0x00,0xA9,0x01,0x85,0x01,0xA2,0x00,0xB5,0x00,0x18,0x75,0x01,0x95,0x02,0xE8,0x90,0xF6,0xE8,0xDB};
	// byte test_program[40] = { 0x18, 0xA9, 0x02, 0x69, 0x02, 0x85, 0x00, 0xDB };
	size_t program_length = sizeof(test_program) / sizeof(byte);

	reset_6502();
	init_memory();
	load_program(0x0800, test_program, program_length);

	run_6502();

	hexdump();

	// for (int i = 0; i < 16; i++) { printf("Memory[%X] : %d\n", i, memory[i]); }
}
