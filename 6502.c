#include <stdint.h>
#include <stdio.h>

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
#define set_carry() status_reg |= FLAG_CARRY
#define set_zero() status_reg |= FLAG_ZERO
#define set_interrupt() status_reg |= FLAG_INTERRUPT
#define set_decimal() status_reg |= FLAG_DECIMAL
#define set_break() status_reg |= FLAG_BREAK
#define set_constant() status_reg |= FLAG_CONSTANT
#define set_overflow() status_reg |= FLAG_OVERFLOW
#define set_sign() status_reg |= FLAG_SIGN
#define clear_carry() status_reg &= ~FLAG_CARRY
#define clear_zero() status_reg &= ~FLAG_ZERO
#define clear_interrupt() status_reg &= ~FLAG_INTERRUPT
#define clear_decimal() status_reg &= ~FLAG_DECIMAL
#define clear_break() status_reg &= ~FLAG_BREAK
#define clear_constant() status_reg &= ~FLAG_CONSTANT
#define clear_overflow() status_reg &= ~FLAG_OVERFLOW
#define clear_sign() status_reg &= ~FLAG_SIGN

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
	instruction_ptr = ((word)read_6502(0xFFFC) | ((word)read_6502(0xFFFD) << 8));
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


//  INFO: FLAG FUNCTIONS

// Zero
static void zero_check(word input_value) {
	if (input_value & 0x00FF) clear_zero();
	else set_zero();
}

// Sign
static void sign_check(word input_value) {
	if (input_value & 0x0080) set_sign();
	else clear_sign();
}

// Carry
static void carry_check(word input_value) {
	if (input_value & 0xFF00) set_carry();
	else clear_carry();
}

// Overflow
static void overflow_check(word input_value, word input_result) {
	if (((accumulator ^ input_result) & FLAG_SIGN) & ((input_value ^ input_result) & FLAG_SIGN))
		set_overflow();
	else clear_overflow();
}


//  INFO: ADDRESSING MODES

static void imp() { /* implied */ }
static void acc() { /* accumulator */ }

//  FIX: IF STATEMENT BUG, STORE ACCUMULATOR DOESNT WORK
static byte get_value() {
	if (address_table[opcode] == acc) return (word)accumulator;
	else return (word)read_6502(effective_addr);
}

static word get_value_word() {
	return ((word)read_6502(effective_addr) | ((word)read_6502(effective_addr + 1) << 8));
}

//  FIX: IF STATEMENT BUG, STORE ACCUMULATOR DOESNT WORK
static void put_value(word input_value) {
	if (address_table[opcode] == acc) {  accumulator = (byte)input_value; }
	else write_6502(effective_addr, (byte)input_value);
}

static void imm() {		// Immediate
	effective_addr = instruction_ptr++; 
}

static void absl() {		// Absolute
	effective_addr = (word)read_6502(instruction_ptr) | (word)(read_6502(instruction_ptr+1) << 8);
	instruction_ptr += 2;
}

static void zrp() {		// Zero-Page
	effective_addr = (word)(read_6502(instruction_ptr));
	instruction_ptr++;
}

static void absx() {	// Absolute-X
	effective_addr = (word)read_6502(instruction_ptr) | (word)(read_6502(instruction_ptr+1) << 8);
	effective_addr = (effective_addr + x_reg);
	instruction_ptr += 2;
}

static void absy() {	// Absolute-Y
	effective_addr = (word)read_6502(instruction_ptr) | (word)(read_6502(instruction_ptr+1) << 8);
	effective_addr = (effective_addr + y_reg);
	instruction_ptr += 2;
}

static void zrpx() {	// Zero Page-X
	effective_addr = ((word)read_6502(instruction_ptr) + x_reg) & 0xFF;
	instruction_ptr++;
}

static void zrpy() {	// Zero Page-Y
	effective_addr = ((word)read_6502(instruction_ptr) + y_reg) & 0xFF;
	instruction_ptr++;
}

static void ind() {		// Indirect 	  FIX: Missing page wrap-around bug
	word temp_addr = (word)read_6502(instruction_ptr) | (word)(read_6502(instruction_ptr+1) << 8);
	effective_addr = (word)read_6502(temp_addr) | (word)(read_6502(temp_addr+1) << 8);
	instruction_ptr += 2;
}

static void indx() {	// Indirect-X
	word temp_addr = ((word)read_6502(instruction_ptr) + x_reg) & 0xFF;
	effective_addr = (word)read_6502(temp_addr) | ((word)read_6502(temp_addr+1) << 8);
	instruction_ptr++;
}

static void indy() {	// Indirect-Y
	word temp_addr = read_6502(instruction_ptr) & 0xFF;
	effective_addr = (((word)read_6502(temp_addr) | ((word)read_6502(temp_addr+1) << 8)) + y_reg);
	instruction_ptr++;
}

static void rel() {		// Relative
	relative_addr = (word)read_6502(instruction_ptr);
	// checking if negative and convert to negative 16-bit signed int
	if (relative_addr & 0x80) { relative_addr |= 0xFF00; }
	instruction_ptr++;
}


//  INFO: OPCODE FUNCTIONS

// Add Memory to Accumulator with Carry
static void adc() {
	value = get_value();
	result = (word)accumulator + value + (word)(status_reg & FLAG_CARRY);
	zero_check(result); sign_check(result); carry_check(result); overflow_check(value, result);
	accumulator = (byte)result;
}

// And Memeory with Accumulator
static void and() {
	value = get_value();
	result = value & (word)accumulator;
	sign_check(result); zero_check(result);
	accumulator = (byte)result;
}

// Left Bit Shift
static void asl() {
	value = get_value();
	result = value << 1;
	sign_check(result); zero_check(result); carry_check(result);
	put_value(result);
}

//  TODO: Branch ops should check if crossing page boundary

// Branch on Carry Clear
static void bcc() {
	if (!(status_reg & FLAG_CARRY)) instruction_ptr += relative_addr; }

// Branch on Carry Set
static void bcs() {
	if (status_reg & FLAG_CARRY) instruction_ptr += relative_addr; }

// Branch on Zero Result
static void beq() {
	if (status_reg & FLAG_ZERO) instruction_ptr += relative_addr; }

static void bit() { 
	value = get_value();
	status_reg = (status_reg & 0x3F) | (value & 0xC0);
	if (accumulator == (byte)value) set_zero(); else clear_zero();
}

// Branch on Negative Result
static void bmi() {
	if (status_reg & FLAG_SIGN) { instruction_ptr += relative_addr; } }

// Branch on Non-Zero Result
static void bne() {
	if (!(status_reg & FLAG_ZERO)) { instruction_ptr += relative_addr; } }

// Branch on Positive Result
static void bpl() {
	if (!(status_reg & FLAG_SIGN)) { instruction_ptr += relative_addr; } }

static void brk() {
	push_word(instruction_ptr + 2);
	push_byte(status_reg);
	instruction_ptr = ((word)read_6502(0xFFFE) & (word)(read_6502(0xFFFF) << 8));
	set_interrupt();
}

// Branch on Overflow Clear
static void bvc() {
	if (!(status_reg & FLAG_OVERFLOW)) { instruction_ptr += relative_addr; } }

// Branch on Overflow Set
static void bvs() {
	if (status_reg & FLAG_OVERFLOW) { instruction_ptr += relative_addr; } }

// Clear Carry Flag
static void clc() {
	clear_carry(); }

// Clear Decimal Flag
static void cld() {
	clear_decimal(); }

// Clear Interrupt Flag
static void cli() {
	clear_interrupt(); }

// Clear Overflow Flag
static void clv() {
	clear_overflow(); }

// Compare Memory with Accumulator
static void cmp() {
	value = get_value();
	if (accumulator >= (byte)value) set_carry(); else clear_carry();
	if (accumulator == (byte)value) set_zero();  else clear_zero();
	sign_check(result);
}

// Compare Memory with X-Register
static void cpx() { 
	if (x_reg >= (byte)value) set_carry(); else clear_carry();
	if (x_reg == (byte)value) set_zero();  else clear_zero();
	sign_check(result);
}

// Compare Memory with Y-Register
static void cpy() {
	if (y_reg >= (byte)value) set_carry(); else clear_carry();
	if (y_reg == (byte)value) set_zero();  else clear_zero();
	sign_check(result);
}

// Decrement Memory
static void dec() {
	value = get_value();
	result = value - 1;
	sign_check(result); zero_check(result);
	put_value(result);
}

// Decrement X-Register
static void dex() {
	x_reg--; 
	sign_check(x_reg); zero_check(x_reg);
}

// Decrement Y-Register
static void dey() {
	y_reg--;
	sign_check(y_reg); zero_check(y_reg);
}

// XOR Memory with Accumulator
static void eor() {
	value = get_value();
	result = value ^ (word)accumulator;
	sign_check(result); zero_check(result);
	accumulator = (byte)result;
}

// Increment Memory
static void inc() {
	value = get_value();
	result = value + 1;
	sign_check(result); zero_check(result);
	put_value(result);
}

// Incrememnt X-Register
static void inx() {
	x_reg++;
	sign_check(x_reg); zero_check(x_reg);
}

// Increment Y-Register
static void iny() {
	y_reg++;
	sign_check(y_reg); zero_check(y_reg);
}

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
	sign_check(value); zero_check(value);
	accumulator = (byte)value;
}

// Load Memory into X-Register
static void ldx() {
	value = get_value();
	sign_check(value); zero_check(value);
	x_reg = (byte)value;
}

// Load Memory into Y-Register
static void ldy() {
	value = get_value();
	sign_check(value); zero_check(value);
	y_reg = (byte)value;
}

// Right Bit Shift
static void lsr() {
	value = get_value();
	result = value >> 1;
	clear_sign(); zero_check(result); carry_check(result);
	put_value(result);
}

// No Operation
static void nop() { /* Do Nothing */ } 

// Or Memory with Accumulator
static void ora() {
	value = get_value();
	result = value | (word)accumulator;
	sign_check(result); zero_check(result);
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
	accumulator = pull_byte();
	sign_check(accumulator); zero_check(accumulator);
}

// Pull Status
static void plp() {
	status_reg = pull_byte() | FLAG_BREAK; }

// Rotate Left
static void rol() {
	value = get_value();
	result = (value << 1) + (value | 0x80);
	sign_check(result); zero_check(result); carry_check(result);
	put_value(result);
}

// Rotate Right
static void ror() { 
	value = get_value();
	if (value & 0x01) value += 0x100;
	result = (value >> 1);
	sign_check(result); zero_check(result); carry_check(result);
	put_value(result);
}

// Return from Interrupt
static void rti() { 
	status_reg = pull_byte();
	instruction_ptr = pull_word();
}

// Return from Sub-Routine
static void rts() {
	instruction_ptr = pull_word() + 1; }

// Subtract Memory from Accumulator with Carry
static void sbc() {
	value = get_value() & 0x00FF;
	result = (word)accumulator + value + (status_reg & FLAG_CARRY);
	sign_check(result); zero_check(result); carry_check(result); overflow_check(value, result);
	put_value(result);
}

// Set Carry Flag
static void sec() {	set_carry(); }

// Set Decimal Flag
static void sed() { set_decimal(); }

// Set Interrupt Flag
static void sei() { set_interrupt(); }

// Store Accumulator to Memory
static void sta() {
	put_value(accumulator); }

// Stop Processor
static void stp() { /* stop */ }

// Store X-Register to Memory
static void stx() {
	put_value(x_reg); }

// Store Y-Register to Memory
static void sty() {
	put_value(y_reg); }

// Transfer Accumulator to X-Register
static void tax() {
	x_reg = accumulator; 
	sign_check(x_reg); zero_check(x_reg);
}

// Transfer Accumulator to Y-Register
static void tay() {
	y_reg = accumulator;
	sign_check(y_reg); zero_check(y_reg);
}

// Transfer Stack Pointer to X-Register
static void tsx() {
	x_reg = stack_ptr;
	sign_check(x_reg); zero_check(x_reg);
}

// Transfer X-Register to Accumulator
static void txa() {
	accumulator = x_reg;
	sign_check(x_reg); zero_check(x_reg);
}

// Transfer X-Register to Stack Pointer
static void txs() {
	stack_ptr = x_reg; }

// Transfer Y-Register to Accumulator
static void tya() {
	accumulator = y_reg;
	sign_check(y_reg); zero_check(y_reg);
}


//  INFO: FUNCTION TABLES

static void (*opcode_table[256])() = { 
/*		 0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  A  |  B  |  C  |  D  |  E  |  F  */
/* 0 */ brk,  ora,  nop,  nop,  nop,  ora,  asl,  nop,  php,  ora,  asl,  nop,  nop,  ora,  asl,  nop,
/* 1 */ bpl,  ora,  nop,  nop,  nop,  ora,  asl,  nop,  clc,  ora,  nop,  nop,  nop,  ora,  asl,  nop,
/* 2 */ jsr,  and,  nop,  nop,  bit,  and,  rol,  nop,  plp,  and,  rol,  nop,  bit,  and,  rol,  nop,
/* 3 */ bmi,  and,  nop,  nop,  nop,  and,  rol,  nop,  sec,  and,  nop,  nop,  nop,  and,  rol,  nop,
/* 4 */ rti,  eor,  nop,  nop,  nop,  eor,  lsr,  nop,  pha,  eor,  lsr,  nop,  jmp,  eor,  lsr,  nop,
/* 5 */ bvc,  eor,  nop,  nop,  nop,  eor,  lsr,  nop,  cli,  eor,  nop,  nop,  nop,  eor,  lsr,  nop,
/* 6 */ rts,  adc,  nop,  nop,  nop,  adc,  ror,  nop,  pla,  adc,  ror,  nop,  jmp,  adc,  ror,  nop,
/* 7 */ bvs,  adc,  nop,  nop,  nop,  adc,  ror,  nop,  sei,  adc,  nop,  nop,  nop,  adc,  ror,  nop,
/* 8 */ nop,  sta,  nop,  nop,  sty,  sta,  stx,  nop,  dey,  nop,  txa,  nop,  sty,  sta,  stx,  nop,
/* 9 */ bcc,  sta,  nop,  nop,  sty,  sta,  stx,  nop,  tya,  sta,  txs,  nop,  nop,  sta,  nop,  nop, 
/* A */ ldy,  lda,  ldx,  nop,  ldy,  lda,  ldx,  nop,  tay,  lda,  tax,  nop,  ldy,  lda,  ldx,  nop,
/* B */ bcs,  lda,  nop,  nop,  ldy,  lda,  ldx,  nop,  clv,  lda,  tsx,  nop,  ldy,  lda,  ldx,  nop,
/* C */ cpy,  cmp,  nop,  nop,  cpy,  cmp,  dec,  nop,  iny,  cmp,  dex,  nop,  cpy,  cmp,  dec,  nop,
/* D */ bne,  cmp,  nop,  nop,  nop,  cmp,  dec,  nop,  cld,  cmp,  nop,  stp,  nop,  cmp,  dec,  nop,
/* E */ cpx,  sbc,  nop,  nop,  cpx,  sbc,  inc,  nop,  inx,  sbc,  nop,  nop,  cpx,  sbc,  inc,  nop,
/* F */ beq,  sbc,  nop,  nop,  nop,  sbc,  inc,  nop,  sed,  sbc,  nop,  nop,  nop,  sbc,  inc,  nop
};

static void (*address_table[256])() = {
/*		 0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  A  |  B  |  C  |  D  |  E  |  F  */
/* 0 */ imp, indx,  imp,  imp,  imp,  zrp,  zrp,  imp,  imp,  imm,  acc,  imp,  imp, absl, absl,  imp,
/* 1 */ rel, indy,  imp,  imp,  imp, zrpx, zrpx,  imp,  imp, absy,  imp,  imp,  imp, absx, absx,  imp,
/* 2 */ absl,indx,  imp,  imp,  zrp,  zrp,  zrp,  imp,  imp,  imm,  acc,  imp, absl, absl, absl,  imp,
/* 3 */ rel, indy,  imp,  imp,  imp,  zrp,  zrp,  imp,  imp, absy,  imp,  imp,  imp, absx, absx,  imp,
/* 4 */ imp, indx,  imp,  imp,  imp,  zrp,  zrp,  imp,  imp,  imm,  acc,  imp, absl, absl, absl,  imp,
/* 5 */ rel, indy,  imp,  imp,  imp, zrpx, zrpx,  imp,  imp, absy,  imp,  imp,  imp, absx, absx,  imp,
/* 6 */ imp, indx,  imp,  imp,  imp,  zrp,  zrp,  imp,  imp,  imm,  acc,  imp,  ind, absl, absl,  imp,
/* 7 */ rel, indy,  imp,  imp,  imp, zrpx, zrpx,  imp,  imp, absy,  imp,  imp,  imp, absx, absx,  imp,
/* 8 */ imp, indx,  imp,  imp,  zrp,  zrp,  zrp,  imp,  imp,  imp,  imp,  imp, absl, absl, absl,  imp,
/* 9 */ rel, indy,  imp,  imp, zrpx, zrpx, zrpy,  imp,  imp, absy,  imp,  imp,  imp, absx,  imp,  imp,
/* A */ imm, indx,  imm,  imp,  zrp,  zrp,  zrp,  imp,  imp,  imm,  imp,  imp, absl, absl, absl,  imp,
/* B */ rel, indy,  imp,  imp, zrpx, zrpx, zrpy,  imp,  imp, absy,  imp,  imp, absx, absx, absy,  imp,
/* C */ imm, indx,  imp,  imp,  zrp,  zrp,  zrp,  imp,  imp,  imm,  imp,  imp, absl, absl, absl,  imp,
/* D */ rel, indy,  imp,  imp,  imp, zrpx, zrpx,  imp,  imp, absy,  imp,  imp,  imp, absx, absx,  imp,
/* E */ imm, indx,  imp,  imp,  zrp,  zrp,  zrp,  imp,  imp,  imm,  imp,  imp, absl, absl, absl,  imp,
/* F */ rel, indy,  imp,  imp,  imp, zrpx, zrpx,  imp,  imp, absy,  imp,  imp,  imp, absx, absx,  imp
};


// Perform 1 instruction cycle
static void step_6502() {
	opcode = read_6502(instruction_ptr++);
	set_constant();

	(*address_table[opcode])();
	(*opcode_table[opcode])();
}

static void run_6502() {
	while (opcode_table[opcode] != stp) {
		opcode = read_6502(instruction_ptr++);
		set_constant();
		
		(*address_table[opcode])();
		(*opcode_table[opcode])();
	}
}

