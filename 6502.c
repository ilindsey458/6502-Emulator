#include <stdio.h>
#include <stdint.h>

typedef uint8_t byte;
typedef uint16_t word;

// Status flags
#define FLAG_CARRY 		0x01
#define FLAG_ZERO 		0x02
#define FLAG_INTERRUPT 	0x04
#define FLAG_DECIMAL 	0x08
#define FLAG_BREAK 		0x10
#define FLAG_CONSTANT 	0x20
#define FLAG_OVERFLOW 	0x40
#define FLAG_SIGN 		0x80

// Flag Macros
#define setcarry() status |= FLAG_CARRY
#define setzero() status |= FLAG_ZERO
#define setinterrupt() status |= FLAG_INTERRUPT
#define setdecimal() status |= FLAG_DECIMAL
#define setbreak() status |= FLAG_BREAK
#define setconstant() status |= FLAG_CONSTANT
#define setoverflow() status |= FLAG_OVERFLOW
#define setsign() status |= FLAG_SIGN
#define clearcarry() status &= ~FLAG_CARRY
#define clearzero() status &= ~FLAG_ZERO
#define clearinterrupt() status &= ~FLAG_INTERRUPT
#define cleardecimal() status &= ~FLAG_DECIMAL
#define clearbreak() status &= ~FLAG_BREAK
#define clearconstant() status &= ~FLAG_CONSTANT
#define clearoverflow() status &= ~FLAG_OVERFLOW
#define clearsign() status &= ~FLAG_SIGN

#define STACK_START 0xFD
#define STACK_BASE 0x100

// External Memory Functions
extern byte read_6502(word address);
extern void write_6502(word address, byte p_value);

// Registers
byte a, x, y, sp, status;
word pc;

// Variables
byte opcode;
word ea, result, value;

// Processor Functions

// Reset Registers
void reset_6502() {
	a = x = y = 0;
	sp = STACK_START;
}

// Stack Functions
void push_byte(byte p_value) {
	write_6502(STACK_BASE + sp--, p_value);
}

void push_word(word p_value) {
	write_6502(STACK_BASE + sp, (p_value >> 8) & 0xFF);
	write_6502(STACK_BASE + ((sp - 1) & 0xFF), p_value & 0xFF);
	sp -= 2;
}

byte pull_byte() {
	return read_6502(STACK_BASE + ++sp);
}

word pull_word() {
	word output_word;
	output_word = read_6502(STACK_BASE + ((sp + 1) & 0xFF));
	output_word += read_6502(STACK_BASE + ((sp + 2) & 0xFF)) << 8;
	return output_word;
}

// Add Memory to Accumulator with Carry
// A + M + C -> A, C
static void adc() {
	result = (word)a + value + (word)(status & FLAG_CARRY);
}

static void nop() { 
	// Do Nothing
} 



// static void (opcode)() = {
// 	// OPCODES
// }

// void step6502() {
	// instruction = fetch(pc++);
	
	// decode(instruction);

	// execute(instruction);
// }

int main() {
	reset_6502();
	push_byte(65);
	push_byte(20);
	push_word(0xFF55);
	printf("Bottom of stack : %X\n", pull_word());
}
