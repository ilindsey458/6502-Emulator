# MOS6502 Emulator in C
---
This is my C implementation of the MOS6502 Microprocessor. The code is written to be readable and understandable.

## Why make a 6502 Emulator?
This is a learning project to become better at C and understanding hardware logic / embedded systems. This a functional emulation of a 6502 microprocessor that I plan to use in future projects like an NES or Apple 2 emulator. I completed this project to deepen my understanding of low-level programming and system design.

## Installation
Run `git clone https://github.com/ilindsey458/6502-Emulator && cd 6502-Emulator && make`

## Usage
To use this program visit https://www.masswerk.at/6502/assembler.html and write your 6502 assembly program. **MAKE SURE TO ADD** `.org $8000` to the beginning of your assembly program or this emulator will not work. Once your program is assembled download the binary in the default format. Your file should look like 'my_assembly_program.bin'. Then run `./6502 my_assembly_program.bin`

#### Flags
- h - help message
- x - hexdump memory
- d \[instruction count\]- debug mode \[run program for this many instructions\]

## Features
- Registers
- Stack Functions
- OPCode table
- Address mode table
- Interrupts
- FDE cycle
- Memory emulator

#### Future Features (scope creep)
- Cycle emulation
- GUI
- NES mode

## Future Plans
I would like to add a GUI to this application to allow users to see the register and memory contents updating in real time, as well as showing the information about what instruction is being run and such. I would also love to turn this into a proper NES emulator to allow me to play NES games while viewing the internals of the emulated NES. The GUI inspiration is from these amazing projects [macmade/6502](https://github.com/macmade/MOS-6502-Emulator) and [onelonecoder/6502](https://github.com/OneLoneCoder/olcNES)  

#### Inspiration and References
- https://www.masswerk.at/6502/6502_instruction_set.html
- https://github.com/OneLoneCoder/olcNES
- https://github.com/macmade/MOS-6502-Emulator
- https://rubbermallet.org/fake6502.c 

