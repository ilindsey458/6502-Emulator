#include <getopt.h>
#include "6502.c"
#include "debug.c"
#include "memory_unit.c"
#include "helper.c"


int main(int argc, char **argv) {
	args_t args = {0};
	parse_args(argc, argv, &args);

	if (args.help_flag == 1) { help_msg(); return EXIT_SUCCESS; }

	init_memory();
	if (load_program(args.input_file)) return EXIT_FAILURE;
	reset_6502();
	
	if (args.debug_flag == 1) run_debug_6502(args.debug_count);
	else run_6502();

	if (args.hexdump_flag == 1) hexdump();

	return EXIT_SUCCESS;
}
