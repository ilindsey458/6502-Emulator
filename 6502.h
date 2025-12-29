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
word instruction_ptr;

// Variables
byte opcode;
word effective_addr, relative_addr, result, value;

// Tables
static void (*address_table[256])();
static void (*opcode_table[256])();

// Reset Registers
void reset_6502() {
	accumulator = x_reg = y_reg = 0;
	stack_ptr = STACK_START;
	instruction_ptr = 0;				 // TODO: This needs to be the hardcoded start addr value
}


//  INFO: STACK FUNCTIONS

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


//  INFO: ADDRESSING MODES

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

static void imm() {		// Immediate
	effective_addr = instruction_ptr; 
}

static void absl() { 	// Absolute
	effective_addr = (word)read_6502(instruction_ptr) | (word)(read_6502(instruction_ptr+1) << 8);
}

static void zrp() {		// Zero-Page
	effective_addr = (word)(read_6502(instruction_ptr) & 0xFF);
}

static void absx() {	// Absolute-X
	effective_addr = (word)read_6502(instruction_ptr) | (word)(read_6502(instruction_ptr+1) << 8);
	effective_addr = (effective_addr + x_reg);
}

static void absy() {	// Absolute-Y
	effective_addr = (word)read_6502(instruction_ptr) | (word)(read_6502(instruction_ptr+1) << 8);
	effective_addr = (effective_addr + y_reg);
}

static void zrpx() {	// Zero Page-X
	effective_addr = ((word)read_6502(instruction_ptr) + x_reg) & 0xFF;
}

static void zrpy() {	// Zero Page-Y
	effective_addr = ((word)read_6502(instruction_ptr) + y_reg) & 0xFF;
}

static void ind() {		// Indirect 	  FIX: Missing page wrap-around bug
	word temp_addr = read_6502(instruction_ptr) | (read_6502(instruction_ptr+1) << 8);
	effective_addr = read_6502(temp_addr) | (read_6502(temp_addr+1) << 8);
}

static void indx() {	// Indirect-X
	word temp_addr = (read_6502(instruction_ptr) + x_reg) & 0xFF;
	effective_addr = read_6502(temp_addr) | (read_6502(temp_addr+1) << 8);
}

static void indy() {	// Indirect-Y
	word temp_addr = read_6502(instruction_ptr) & 0xFF;
	effective_addr = ((read_6502(temp_addr) | (read_6502(temp_addr+1) << 8)) + y_reg);
}

static void rel() {		// Relative
	relative_addr = (word)read_6502(instruction_ptr);
	// checking if negative and convert to negative 16-bit signed int
	if (relative_addr & 0x80) { relative_addr |= 0xFF00; }
}


//  INFO: OPCODE FUNCTIONS

//  TODO: Need to add all flag checks

// Add Memory to Accumulator with Carry
static void adc() {
	value = get_value();
	result = (word)accumulator + value + (word)(status_reg & FLAG_CARRY);
	accumulator = (byte)result;
}

// And Memeory with Accumulator
static void and() {
	value = get_value();
	result = value & (word)accumulator;
	accumulator = (byte)result;
}

// Left Bit Shift
static void asl() {
	value = get_value();
	result = value << 1;
	put_value(result);
}

//  TODO: Branch ops should check if crossing page boundary

// Branch on Carry Clear
static void bcc() {
	if (!(status_reg & FLAG_CARRY)) { instruction_ptr += relative_addr; } }

// Branch on Carry Set
static void bcs() {
	if (status_reg & FLAG_CARRY) { instruction_ptr += relative_addr; } }

// Branch on Zero Result
static void beq() {
	if (status_reg & FLAG_ZERO) { instruction_ptr += relative_addr; } }

//  TODO: Complicated Bit Test...
static void bit() { }

// Branch on Negative Result
static void bmi() {
	if (status_reg & FLAG_SIGN) { instruction_ptr += relative_addr; } }

// Branch on Non-Zero Result
static void bne() {
	if (!(status_reg & FLAG_ZERO)) { instruction_ptr += relative_addr; } }

// Branch on Positive Result
static void bpl() {
	if (!(status_reg & FLAG_SIGN)) { instruction_ptr += relative_addr; } }

//  TODO: Force Break
static void brk() { }

// Branch on Overflow Clear
static void bvc() {
	if (!(status_reg & FLAG_OVERFLOW)) { instruction_ptr += relative_addr; } }

// Branch on Overflow Set
static void bvs() {
	if (status_reg & FLAG_OVERFLOW) { instruction_ptr += relative_addr; } }

// Clear Carry Flag
static void clc() {
	status_reg &= ~FLAG_CARRY; }

// Clear Decimal Flag
static void cld() {
	status_reg &= ~FLAG_DECIMAL; }

// Clear Interrupt Flag
static void cli() {
	status_reg &= ~FLAG_INTERRUPT; }

// Clear Overflow Flag
static void clv() {
	status_reg &= ~FLAG_OVERFLOW; }

// Compare Memory with Accumulator
//  TODO: Set SIGN / ZERO / CARRY flags
static void cmp() {
	value = get_value();
}

// Compare Memory with X-Register
static void cpx() { }

// Compare Memory with Y-Register
static void cpy() { }

// Decrement Memory
static void dec() {
	value = get_value();
	result = value - 1;
	put_value(result);
}

// Decrement X-Register
static void dex() {
	x_reg--; }

// Decrement Y-Register
static void dey() {
	y_reg--; }

// XOR Memory with Accumulator
static void eor() {
	value = get_value();
	result = value ^ (word)accumulator;
	accumulator = (byte)result;
}

// Increment Memory
static void inc() {
	value = get_value();
	result = value + 1;
	put_value(result);
}

// Incrememnt X-Register
static void inx() {
	x_reg++; }

// Increment Y-Register
static void iny() {
	y_reg++; }

// Jump to Address
static void jmp() {
	instruction_ptr = effective_addr; }

// Jump to SubRoutine
static void jsr() {
	push_word(instruction_ptr - 1);
	instruction_ptr = effective_addr;
}

// Load Memory into Accumulator
static void lda() {
	value = get_value();
	accumulator = (byte)value;
}

// Load Memory into X-Register
static void ldx() {
	value = get_value();
	x_reg = (byte)value;
}

// Load Memory into Y-Register
static void ldy() {
	value = get_value();
	y_reg = (byte)value;
}

// Right Bit Shift
static void lsr() {
	value = get_value();
	result = value >> 1;
	put_value(result);
}

// No Operation
static void nop() { /* Do Nothing */ } 

// Or Memory with Accumulator
static void ora() {
	value = get_value();
	result = value | (word)accumulator;
	put_value(result);
}

// Push Accumulator
static void pha() {
	push_byte(accumulator); }

// Push Status
static void php() {
	push_byte(status_reg | FLAG_BREAK); }

// Pull Accumulator
static void pla() {
	accumulator = pull_byte(); }

// Pull Status
static void plp() {
	status_reg = pull_byte() | FLAG_BREAK; }

// Rotate Left
static void rol() {
	value = get_value();
	result = (value << 1) + (value | 0x80);
	put_value(result);
}

// Rotate Right
static void ror() { 
	value = get_value();
	if (value & 0x01) value += 0x100;
	result = (value >> 1);
	put_value(result);
}

// Return from Interrupt
static void rti() { }

// Return from Sub-Routine
static void rts() {
	instruction_ptr = pull_word() + 1; }

// Subtract Memory from Accumulator with Carry
static void sbc() {
	value = get_value();
	//  FIX: This doesn't work yet but I'm tired...
	// result = (word)accumulator + ~value + (status_reg & FLAG_CARRY);
}


//  INFO: FUNCTION TABLES

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
