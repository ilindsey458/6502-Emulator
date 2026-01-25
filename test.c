#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "6502.c"

#define MEM_SIZE 0x10000
#define HEX_DUMP_WIDTH 8
#define MAX_BINARY_SIZE 0x7000
#define PROGRAM_START 0x8000

static byte memory[MEM_SIZE];

typedef struct {
	char *input_file;
	int hexdump_flag;
} args_t;

// Set all elements of array to 0
void init_memory() {
	for (int i = 0; i < MEM_SIZE; i++) { memory[i] = 0; }
}

// Returns byte at address in memory
byte read_6502(word address) {
	return memory[address];
}

// Writes byte to address in memory
void write_6502(word address, byte p_value) {
	memory[address] = p_value;
}

void parse_args(int argc, char **argv, args_t *args) {
	int opt;
	do  {
		switch(opt = getopt(argc, argv, ":x")) {
		case -1:
			args->input_file = argv[optind];
			break;
		case 'x':
			args->hexdump_flag = 1;
			break;
		default:
			break;
		}
	} while (opt >= 0);
}

//  INFO: Prints non-NULL memory space
static void hexdump() {
	int line_start;
	for (int i = 0; i < MEM_SIZE; i++) {
		if (memory[i]) {
			line_start = i - (i % HEX_DUMP_WIDTH);
			printf("%X : ", line_start);
			for (int j = 0; j < HEX_DUMP_WIDTH - 1; j++) { printf("%X ", memory[line_start+j]); }
			printf("%X\n", memory[line_start + (HEX_DUMP_WIDTH - 1)]);
			i += (HEX_DUMP_WIDTH - 1); }
	}
}

void chip_out() { printf("Should do output\n"); }

//  INFO: Loads binary file into memory
int load_program(char *file_name) {
	FILE *file_ptr = fopen(file_name, "rb");
	if (file_ptr == NULL) { 
		fprintf(stderr, "Error reading %s\n", file_name);
		return EXIT_FAILURE; }

	if (fseek(file_ptr, 0, SEEK_END)) {
		perror("Error reading file size\n");
		fclose(file_ptr);
		return EXIT_FAILURE; }

	size_t file_size = ftell(file_ptr);
	if (file_size == -1) { 
		perror("Error determining file position\n");
		fclose(file_ptr);
		return EXIT_FAILURE; }
	else if (file_size > MAX_BINARY_SIZE) {
		perror("Error, file size too large\n");
		fclose(file_ptr);
		return EXIT_FAILURE; }

	printf("Size of file in bytes : %ld\n", file_size);
	rewind(file_ptr);

	fread(memory + PROGRAM_START, 1, file_size, file_ptr);
	memory[PROGRAM_START + file_size] = 0xDB;
	for (int i = 0; i <= file_size; i++) {
		if (i == file_size) printf("%x\n", memory[i + PROGRAM_START]);
		else printf("%x, ", memory[i + PROGRAM_START]); }

	fclose(file_ptr);
	return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
	args_t args = {0};
	parse_args(argc, argv, &args);

	reset_6502();
	init_memory();
	load_program(args.input_file);

	run_6502();

	if (args.hexdump_flag == 1) hexdump();
}
