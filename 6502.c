#include <stdio.h>
#include <stdint.h>

typedef uint8_t byte;
typedef uint16_t word;

#define FLAG_CARRY 		0x01
#define FLAG_ZERO 		0x02
#define FLAG_INTERRUPT 	0x04
#define FLAG_DECIMAL 	0x08
#define FLAG_BREAK 		0x10
#define FLAG_CONSTANT 	0x20
#define FLAG_OVERFLOW 	0x40
#define FLAG_SIGN 		0x80


byte A, X, Y, SP, status;
word PC;

static void (opcode)() = {
	// OPCODES
}

void step6502() {
	// instruction = fetch(pc++);
	
	// decode(instruction);

	// execute(instruction);
}


int main() {
	byte instruction;
}
