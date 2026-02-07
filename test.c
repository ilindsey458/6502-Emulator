#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include "6502.c"
#include "debug.c"

#define MEM_SIZE 0x10000
#define HEX_DUMP_WIDTH 8
#define MAX_BINARY_SIZE 0x7000
#define PROGRAM_START 0x8000
#define OUTPUT_ADDRESS 0xFFD2		// HACK: Idk if this is correct

static byte memory[MEM_SIZE];

typedef struct {
	char *input_file;
	size_t debug_count;
	bool hexdump_flag;
	bool debug_flag;
	bool help_flag;
} args_t;

//  INFO: Set all elements of array to 0
static void init_memory() {
	for (int i = 0; i < MEM_SIZE; i++) { memory[i] = 0; }
}

//  INFO: Returns byte at address in memory
byte read_6502(word address) {
	return memory[address];
}

//  INFO: Writes byte to address in memory
void write_6502(word address, byte p_value) {
	memory[address] = p_value;
}

//  INFO: Parses command line args and fills args_t
static void parse_args(int argc, char **argv, args_t *args) {
	int opt;
	do  {
		switch(opt = getopt(argc, argv, "xd:h")) {
		case -1:
			args->input_file = argv[optind];
			break;
		case 'x':
			args->hexdump_flag = 1;
			break;
		case 'd':
			args->debug_flag = 1;
			args->debug_count = (size_t)atoi(optarg);
			break;
		case 'h':
			args->help_flag = 1;
			break;
		default:
			args->help_flag = 1;
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
			printf("%04X : ", line_start);
			for (int j = 0; j < HEX_DUMP_WIDTH - 1; j++) { printf("%02X ", memory[line_start+j]); }
			printf("%02X\n", memory[line_start + (HEX_DUMP_WIDTH - 1)]);
			i += (HEX_DUMP_WIDTH - 1); }
	}
}


//  TODO: This should read byte info from OUTPUT_ADDRESS when anything is written there
static void chip_out() { printf("Should do output\n"); }

//  TODO: I need to write this...
static void help_msg() {
	printf("usage : 6502_emu [options] binary_file.bin \n"
			" options : \n"
			"  -d instruction_count\n\truns emulator in debug mode, executing instruction_count number of instructions\n"
			"  -h\n\tdisplays help message\n"
			"  -x\n\tdisplays hex dump of non-null emulated memory\n"
			" example : 6502_emu hello_world.bin -x -d 25\n");
}


//  INFO: Validates input-file is type .bin
static int check_file_type(char *file_name) {
	char *file_type = strrchr(file_name, '.');
	if (file_type && file_type != file_name) file_type += 1;
	else { fprintf(stderr, "Error, wrong file type\n"); return EXIT_FAILURE; }

	if (strcmp(file_type, "bin")) {
		fprintf(stderr, "Error, wrong file type\n");
		return EXIT_FAILURE; }

	return EXIT_SUCCESS;
}

//  INFO: Loads binary file into memory
static int load_program(char *file_name) {
	if (check_file_type(file_name)) return EXIT_FAILURE;

	// Loading file
	FILE *file_ptr = fopen(file_name, "rb");
	if (file_ptr == NULL) { 
		fprintf(stderr, "Error reading %s\n", file_name);
		return EXIT_FAILURE; }

	if (fseek(file_ptr, 0, SEEK_END)) {
		fprintf(stderr, "Error reading file size\n");
		fclose(file_ptr);
		return EXIT_FAILURE; }

	size_t file_size = ftell(file_ptr);
	if (file_size == -1) { 
		fprintf(stderr, "Error determining file position\n");
		fclose(file_ptr);
		return EXIT_FAILURE; }
	else if (file_size > MAX_BINARY_SIZE || file_size == 0) {
		fprintf(stderr, "Error, invalid file size\n");
		fclose(file_ptr);
		return EXIT_FAILURE; }

	rewind(file_ptr);

	// Loading file contents into memory
	fread(&memory[PROGRAM_START], sizeof(byte), file_size, file_ptr);

 	//  HACK: Probably a better way to do this 
	// 		  Adding stop to program end, and setting RST vector to program start
	memory[PROGRAM_START + file_size] = 0xDB;
	memory[RESET_VECTOR] = (byte)PROGRAM_START;
	memory[RESET_VECTOR + 1] = (byte)(PROGRAM_START >> 8);

	fclose(file_ptr);
	return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
	args_t args = {0};
	parse_args(argc, argv, &args);

	if (args.help_flag == 1) { help_msg(); return EXIT_SUCCESS; }

	init_memory();
	load_program(args.input_file);
	reset_6502();

	if (args.debug_flag == 1) run_debug_6502(args.debug_count);
	else run_6502();

	if (args.hexdump_flag == 1) hexdump();

	return EXIT_SUCCESS;
}
