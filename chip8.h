#ifndef _CHIP8_H
#define _CHIP8_H
#include <stdint.h>
#include <stdlib.h>

struct mState {
        uint8_t registers[16];
        uint16_t iRegister;
        int16_t pc;
        int16_t *stack;
        uint8_t disp[32][8];
        size_t stackSize;
        size_t stackCapacity;
        uint8_t mem[4096];
};

struct runtime_error {
        char *msg;
};

void run_instruction(struct mState *ms, uint16_t ins);
struct mState *create_mState(void);
void delete_mState(struct mState **ms);
#endif
