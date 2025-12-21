#include "6502.h"
#include "memory_64k.h"

int main() {
	reset_6502();
	push_byte(65);
	push_byte(20);
	push_word(0xFF55);
	push_word(0x1234);
	printf("Effective Address : %X\n", effective_addr);
	printf("Bottom of stack : %X\n", pull_word());
}
