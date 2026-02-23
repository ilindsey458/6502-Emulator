#pragma once
#include "6502.c"

char *opcode_names[256] = { 
/*		 0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  A  |  B  |  C  |  D  |  E  |  F  */
/* 0 */ "brk","ora","nop","nop","nop","ora","asl","nop","php","ora","asl","nop","nop","ora","asl","nop",
/* 1 */ "bpl","ora","nop","nop","nop","ora","asl","nop","clc","ora","nop","nop","nop","ora","asl","nop",
/* 2 */ "jsr","and","nop","nop","bit","and","rol","nop","plp","and","rol","nop","bit","and","rol","nop",
/* 3 */ "bmi","and","nop","nop","nop","and","rol","nop","sec","and","nop","nop","nop","and","rol","nop",
/* 4 */ "rti","eor","nop","nop","nop","eor","lsr","nop","pha","eor","lsr","nop","jmp","eor","lsr","nop",
/* 5 */ "bvc","eor","nop","nop","nop","eor","lsr","nop","cli","eor","nop","nop","nop","eor","lsr","nop",
/* 6 */ "rts","adc","nop","nop","nop","adc","ror","nop","pla","adc","ror","nop","jmp","adc","ror","nop",
/* 7 */ "bvs","adc","nop","nop","nop","adc","ror","nop","sei","adc","nop","nop","nop","adc","ror","nop",
/* 8 */ "nop","sta","nop","nop","sty","sta","stx","nop","dey","nop","txa","nop","sty","sta","stx","nop",
/* 9 */ "bcc","sta","nop","nop","sty","sta","stx","nop","tya","sta","txs","nop","nop","sta","nop","nop",
/* A */ "ldy","lda","ldx","nop","ldy","lda","ldx","nop","tay","lda","tax","nop","ldy","lda","ldx","nop",
/* B */ "bcs","lda","nop","nop","ldy","lda","ldx","nop","clv","lda","tsx","nop","ldy","lda","ldx","nop",
/* C */ "cpy","cmp","nop","nop","cpy","cmp","dec","nop","iny","cmp","dex","nop","cpy","cmp","dec","nop",
/* D */ "bne","cmp","nop","nop","nop","cmp","dec","nop","cld","cmp","nop","stp","nop","cmp","dec","nop",
/* E */ "cpx","sbc","nop","nop","cpx","sbc","inc","nop","inx","sbc","nop","nop","cpx","sbc","inc","nop",
/* F */ "beq","sbc","nop","nop","nop","sbc","inc","nop","sed","sbc","nop","nop","nop","sbc","inc","nop"
};

char *addr_mode_names[256] = {
"imp","indx","imp","imp","imp","zrp","zrp","imp","imp","imm","acc","imp","imp","absl","absl","imp",
"rel","indy","imp","imp","imp","zrpx","zrpx","imp","imp","absy","imp","imp","imp","absx","absx","imp",
"absl","indx","imp","imp","zrp","zrp","zrp","imp","imp","imm","acc","imp","absl","absl","absl","imp",
"rel","indy","imp","imp","imp","zrp","zrp","imp","imp","absy","imp","imp","imp","absx","absx","imp",
"imp","indx","imp","imp","imp","zrp","zrp","imp","imp","imm","acc","imp","absl","absl","absl","imp",
"rel","indy","imp","imp","imp","zrpx","zrpx","imp","imp","absy","imp","imp","imp","absx","absx","imp",
"imp","indx","imp","imp","imp","zrp","zrp","imp","imp","imm","acc","imp","ind","absl","absl","imp",
"rel","indy","imp","imp","imp","zrpx","zrpx","imp","imp","absy","imp","imp","imp","absx","absx","imp",
"imp","indx","imp","imp","zrp","zrp","zrp","imp","imp","imp","imp","imp","absl","absl","absl","imp",
"rel","indy","imp","imp","zrpx","zrpx","zrpy","imp","imp","absy","imp","imp","imp","absx","imp","imp",
"imm","indx","imm","imp","zrp","zrp","zrp","imp","imp","imm","imp","imp","absl","absl","absl","imp",
"rel","indy","imp","imp","zrpx","zrpx","zrpy","imp","imp","absy","imp","imp","absx","absx","absy","imp",
"imm","indx","imp","imp","zrp","zrp","zrp","imp","imp","imm","imp","imp","absl","absl","absl","imp",
"rel","indy","imp","imp","imp","zrpx","zrpx","imp","imp","absy","imp","imp","imp","absx","absx","imp",
"imm","indx","imp","imp","zrp","zrp","zrp","imp","imp","imm","imp","imp","absl","absl","absl","imp",
"rel","indy","imp","imp","imp","zrpx","zrpx","imp","imp","absy","imp","imp","imp","absx","absx","imp"
};

static void run_debug_6502(size_t instruction_count) {
	for (size_t i = 0; i < instruction_count; i++) {
		opcode = read_6502(instruction_ptr++);
		set_constant();

		printf("IP : %04x | ", instruction_ptr);
		printf("Addr Mode : %4s | ", addr_mode_names[opcode]);
		printf("Opcode : %s | Hex : %02x | ", opcode_names[opcode], opcode);
		printf("A : %02x | X : %02x | Y : %02x | S : %08b\n", accumulator, x_reg, y_reg, status_reg);
		(*address_table[opcode])();
		(*opcode_table[opcode])();
	}
}
