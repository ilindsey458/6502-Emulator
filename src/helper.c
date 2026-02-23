#include "memory_unit.c"
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>

#define HEX_DUMP_WIDTH 8

typedef struct {
	char *input_file;
	size_t debug_count;
	bool hexdump_flag;
	bool debug_flag;
	bool help_flag;
} args_t;

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

//  INFO: Prints help message
static void help_msg() {
	printf("  usage : 6502_emu binary_file.bin [options]\n"
			"  \tTo use this program visit https://www.masswerk.at/6502/assembler.html and write your 6502 assembly program. MAKE SURE TO ADD '.org $8000' to the beginning of your assembly program or this emulator will not work. Once your program is assembled download the binary in the default format. Your file should look like 'my_assembly_program.bin'. Then follow usage guide to execute your program.\n\n"
			"  options : \n"
			"    -d [instruction_count]\truns emulator in debug mode, executing instruction_count number of instructions\n"
			"    -h\t\t\t\tdisplays help message\n"
			"    -x\t\t\t\tdisplays hex dump of non-null emulated memory\n\n"
			"  example : 6502_emu hello_world.bin -x -d 25\n"
		    "  \tThis will run the 6502 emulator in debug mode. It will execute 25 instructions and output its register contents as well as the opcode information. A hex dump will also be performed showing non-null memory contents.\n");
}

