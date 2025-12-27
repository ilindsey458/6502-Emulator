#include "6502.h"
#include "chip_mem.h"

int main() {
	reset_6502();
	push_byte(65);
	push_byte(20);
	push_word(0xFF55);
	push_word(0x1234);

	write_6502(program_counter, 0xF9);
	write_6502(program_counter + 1, 0x45);

	program_counter = 0x1000;

	write_6502(0x1000, 0x50);
	write_6502(0x1001, 0x20);
	write_6502(0x50, 0x77);
	write_6502(0x51, 0x66);

	x_reg = 10;
	y_reg = 15;

	indy();

	printf("Effective Address : %X\n", effective_addr);
	printf("Bottom of stack : %X\n", pull_word());
}
