#include "stdio.h"
#include "6502.c"
#include "chip_mem.h"

int main() {
	reset_6502();
	push_byte(65);
	push_byte(20);
	push_word(0xFF55);
	push_word(0x1234);

	write_6502(instruction_ptr, 0xF9);
	write_6502(instruction_ptr + 1, 0x45);

	instruction_ptr = 0x1000;

	write_6502(0x1000, 0x80);
	write_6502(0x1001, 0x20);
	write_6502(0x50, 0x77);
	write_6502(0x51, 0x66);

	x_reg = 10;
	y_reg = 15;
	accumulator = 44;
	word test = 10;

	result = (word)accumulator + (test ^ 0x00FF);
	// result = (word)accumulator + test;

	printf("Test : %d\n", (byte)result);

	printf("Accumulator : %X\n", accumulator);

	printf("Effective Address : %X\n", effective_addr);
	printf("Bottom of stack : %X\n", pull_word());
}
