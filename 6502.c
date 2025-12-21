#include <stdio.h>
#include <stdint.h>

// Bit Type Definitions
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
byte accumulator, x_reg, y_reg, stack_ptr, status_reg;
word program_counter;

// Variables
byte opcode;
word effective_addr, result, value;

// Tables
static void (*address_table[256])();
static void (*opcode_table[256])();


// PROCESSOR FUNCTIONS

// Reset Registers
void reset_6502() {
	accumulator = x_reg = y_reg = 0;
	stack_ptr = STACK_START;
	program_counter = 0;				 // TODO: This needs to be the hardcoded start addr value
}

// Stack Functions
void push_byte(byte p_value) {
	write_6502(STACK_BASE + stack_ptr--, p_value);
}

void push_word(word p_value) {
	write_6502(STACK_BASE + stack_ptr, (p_value >> 8) & 0xFF);
	write_6502(STACK_BASE + ((stack_ptr - 1) & 0xFF), p_value & 0xFF);
	stack_ptr -= 2;
}

byte pull_byte() {
	return read_6502(STACK_BASE + ++stack_ptr);
}

word pull_word() {
	word output_word;
	output_word = read_6502(STACK_BASE + ((stack_ptr + 1) & 0xFF));
	output_word += read_6502(STACK_BASE + ((stack_ptr + 2) & 0xFF)) << 8;
	return output_word;
}

// Addressing Modes

static void imp() { /* implied */ }
static void acc() { /* accumulator */ }

static byte get_value() {
	if (address_table[opcode] == acc) return (word)accumulator;
	else return (word)read_6502(effective_addr);
}

static word get_value_word() {
	return (word)(read_6502(effective_addr) | (read_6502(effective_addr + 1) << 8));
}

static void put_value(word input_value) {
	if (address_table[opcode] == acc) accumulator = (byte)(input_value & 0xFF);
	else write_6502(effective_addr, (byte)(input_value & 0xFF));
}

static void imm() { effective_addr++; }

static void absl() { 
	effective_addr = (word)read_6502(program_counter) | (word)(read_6502(program_counter+1) << 8);
}

static void zrp() { effective_addr = (word)(read_6502(program_counter) & 0xFF); }




// Add Memory to Accumulator with Carry
// A + M + C -> A, C
static void adc() {
	result = (word)accumulator + value + (word)(status_reg & FLAG_CARRY);
}

static void nop() { 
	// Do Nothing
} 


static void (*address_table[256])() = { imp };

static void (*opcode_table[256])() = { acc };

// static void (opcode)() = {
// 	// OPCODES
// }

// void step6502() {
	// instruction = fetch(pc++);
	
	// decode(instruction);

	// execute(instruction);
// }
