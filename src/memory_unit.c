#pragma once

#include "6502.c"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MEM_SIZE 0x10000
#define MAX_BINARY_SIZE 0x7000
#define PROGRAM_START 0x8000


//  INFO: Memory Array
static byte memory[MEM_SIZE];

//  INFO: Set all elements of array to 0
static void init_memory() {
	for (int i = 0; i < MEM_SIZE; i++) { memory[i] = 0; }
}

//  INFO: Returns byte at address in memory
byte read_6502(word address) {
	return memory[address];
}

//  INFO: Writes byte to address in memory
void write_6502(word address, byte p_value) {
	memory[address] = p_value;
}

//  INFO: Validates input-file is type .bin
static int check_file_type(char *file_name) {
	char *file_type = strrchr(file_name, '.');
	if (file_type && file_type != file_name) file_type += 1;
	else { fprintf(stderr, "Error, wrong file type\n"); return EXIT_FAILURE; }

	if (strcmp(file_type, "bin")) {
		fprintf(stderr, "Error, wrong file type\n");
		return EXIT_FAILURE; }

	return EXIT_SUCCESS;
}

//  INFO: Loads binary file into memory
static int load_program(char *file_name) {
	if (check_file_type(file_name)) return EXIT_FAILURE;

	// Loading file
	FILE *file_ptr = fopen(file_name, "rb");
	if (file_ptr == NULL) { 
		fprintf(stderr, "Error reading %s\n", file_name);
		return EXIT_FAILURE; }

	if (fseek(file_ptr, 0, SEEK_END)) {
		fprintf(stderr, "Error reading file size\n");
		fclose(file_ptr);
		return EXIT_FAILURE; }

	size_t file_size = ftell(file_ptr);
	if (file_size == -1) { 
		fprintf(stderr, "Error determining file position\n");
		fclose(file_ptr);
		return EXIT_FAILURE; }
	else if (file_size > MAX_BINARY_SIZE || file_size == 0) {
		fprintf(stderr, "Error, invalid file size\n");
		fclose(file_ptr);
		return EXIT_FAILURE; }

	rewind(file_ptr);

	// Loading file contents into memory
	fread(&memory[PROGRAM_START], sizeof(byte), file_size, file_ptr);

 	//  HACK: Probably a better way to do this 
	// 		  Adding stop to program end, and setting RST vector to program start
	memory[PROGRAM_START + file_size] = 0xDB;
	memory[RESET_VECTOR] = (byte)PROGRAM_START;
	memory[RESET_VECTOR + 1] = (byte)(PROGRAM_START >> 8);

	fclose(file_ptr);
	return EXIT_SUCCESS;
}

