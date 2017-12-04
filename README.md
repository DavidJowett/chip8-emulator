# Chip-8 Emulator
A [Chip-8](https://en.wikipedia.org/wiki/CHIP-8) Emulator written in C with OpenGL. Chip-8 is interpreted programming language from the 1970s originally developed for the COSMAC VIP and Telmac 1800 8-bit computers.

## Virtual Machine Description

Chip-8 VM is composed of 16 8 bit general purpose data registers V0-VF. Register VF is used as a flag for some instructions. Two other special registers exist, the 16 bit address register, I, and the 16 bit Program Counter. The VM contains 4096 1 byte memory locations. Of those, the first 512 bytes are reserved for the interpreter with the program memory starting at address 0x200. Output is via a 64x32 monochromatic display, with input from a 16 key  hex keyboard. Additionally, there are two timers, the delay time and the sound timer, both of which count down at 60 Hz until they reach zero. The sound times produces a tone when it is not zero

## Opcodes
Each instruction is 16bit length and contains some combination of registers IDs, and immediate values. The opcodes are stored big-endian.

* X and Y are 4 bit registers IDs
* I is the memory register
* N is a 4 bit immediate value
* NN is a 8 bit immediate value
* NNN is a 12 bit immediate value

Code | Description | Status
-----|-------------|-------
0NNN | Call RCA 1802 program at address NNN | Not implemented few ROMs depend on it
00E0 | Clear display | Implemented and Tested
00EE | Return from subroutine | Implemented and Tested
1NNN | Jump to address NNN | Implemented and Tested
2NNN | Call subroutine at NNN | Implemented and Tested
3XNN | Skip next instruction if Vx = NN | Implemented and Tested
4XNN | Skip next instruction if Vx != NN | Implemented and Tested
5XY0 | Skip next instruction if Vx == Vy | Implemented and Tested
6XNN | Sets Vx to NN | Implemented and Tested
7XNN | Sets Vx to Vx plus NN | Implemented and Tested
8XY0 | Sets Vx to Vy | Implemented and Tested
8XY1 | Sets Vx to bitwise Vx or Vy | Implemented and Tested
8XY2 | Sets Vx to bitwise Vx and Vy | Implemented and Tested
8XY3 | Sets Vx to bitwise Vx xor Vy | Implemented and Tested
8XY4 | Sets Vx to Vx plus Vy | Implemented and Tested
8XY5 | Sets Vx to Vx minus Vy | Implemented and Tested
8XY6 | Sets Vx and Vy to Vy shifted one bit to the right. Vf is set to the least significant bit of Vy from before the shift | Implemented and Tested
8XY7 | Sets Vx to Vy minus Vx + Implemented and Tested | Implement and Tested
8XYE | Sets Vx and Vy to Vy shifted one bit to the left. Vf is set to the most significant bit of Vy from before the shift | Implemented and Tested
9XY0 | Skips the next instruction if Vx != Vy | Implemented and Tested
ANNN | Sets I to NNN | Implemented and Tested
BNNN | Jumps to Vx + NNN | Implemented and Tested
CXNN | Sets Vx to random [0,255] anded with NN | Implemented and Tested
DXYN | Draws a sprite at (Vx,Vy) with a width of 8 pixels and a height of N pixels from the memory location pointed to by I. Sprites are xor'd with the current state of the screen. Vf is set to 1 if any pixels go from set to unset | Implemented and Tested
EX9E | Skips the next instruction if the key code in Vx is pressed | Implemented and Tested
EXA1 | Skips the next instruction if the key code in Vx is not pressed | Implemented and Tested
FX07 | Sets Vx to the current value of the delay timer | Implemented and Tested
FX0A | Blocks until the next key press, and stores the key code in Vx | Implemented and Tested
FX15 | Sets the delay timer to Vx | Implemented and Tested
FX18 | Sets the sound timer to Vx | Implemented and Tested
FX1E | Sets I to I + Vx | Implemented and Tested
FX29 | Sets I to the location of the font character in Vx. For example if Vx 0xA, I will be set to the location of 'A' | Implemented and Tested
FX33 | Causes the BCD representation of Vx to be stored starting at I. I + 0 is set to the hundreds place of Vx, I + 1 is set to the tens place of Vx, and I + 2 is set to the ones place of Vx | Implemented and Tested
FX55 | Dump registers [V0, VX] to memory starting at I. I is incremented for each value written | Implemented and Tested
FX65 | Load registers [V0, VX] from memory starting at I. I is incremented for each value read | Implemented and Tested

## Project Structure
* `src` contains the sources
* `test` contains the unit, and functionality tests
* `testdata` contains test data used in the functionality tests
* `roms` contains public domain ROMS for the Chip-8

## Building
### Executable
1. Build binary
 `make`
### Unit Tests
1. Build unit Tests

 `make test`
2. Execute Unit Tests

 `./tests`

## Unit Tests
The unit tests for the Chip-8 core implementation are located in the `test/chip8_test.c` file along with additional units tests for other components. These tests are written using the [Check](https://libcheck.github.io/check/) C unit testing frame work. The goals of Chip-* core unit tests is to test the core functionality of the Chip-8 VM, by verify each instruction behaves as expected along verifying higher level functionality such as timers, drawing to the display, and program executions works as expected.

## Current Status
The implementation of the emulator core is complete. The display output, and keyboarad input are functioning correctly.

## Todo
1. Key maps
2: Instruction rate limiting
 * Set a fixxed max rate of instructions per a second
3. Sound timer
 * Make a sound when delay timer is not 0
