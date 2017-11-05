#include <stdio.h>
#include <stdint.h>
#include "chip8.h"



static void clear_display(struct mState *ms);

// returns the last 4 bits of ins
static inline int8_t get4bit(int16_t ins){
        return ins & 0xF;
}


static inline uint8_t get8bit(int16_t ins){
        return ins & 0xFF;
}

static inline int16_t get12bit(int16_t ins){
        return (ins & 0xFFF);
}

static inline void getRegister(int16_t ins, uint8_t *reg){
       *reg = (ins >> 8) & 0xF;
}

static inline void get2Registers(int16_t ins, uint8_t *reg1, uint8_t *reg2){
        *reg1 = (ins >> 8) & 0xF;
        *reg2 = (ins >> 4) & 0xF;

}

struct mState *create_mState(void){
        struct mState *ms = malloc(sizeof(struct mState));
        if(ms == NULL) return NULL;
        ms->stack = calloc(48, sizeof(int16_t));
        if(ms->stack == NULL) goto mStateInitFail;
        ms->stackSize = 0;
        ms->stackCapacity = 48;
        ms->pc = 0;
        ms->iRegister = 0;
        for(int i = 0; i < 16; i++)
                ms->registers[i] = 0;
        clear_display(ms);

        return ms;

mStateInitFail:
        free(ms);
        return NULL;
}

void delete_mState(struct mState **ms){
        free((*ms)->stack);
        free(*ms);
        *ms = NULL;
}

static void clear_display(struct mState *ms){
        for(int i = 0; i < 32; i++)
                for(int j = 0; j < 8; j++)
                        ms->disp[i][j] = 0;
}


void run_instruction(struct mState *ms, uint16_t ins){
        uint8_t opc = (ins >> 12);
        switch(opc){
                case 0x0:
                        if(ins == 0x00E0) {
                                clear_display(ms);
                                ms->pc += 2;
                        } else if(ins == 0x00EE){
                                if(ms->stackSize == 0){
                                        puts("return called when stack is empty");
                                } else {
                                        ms->pc = ms->stack[ms->stackSize - 1];
                                        ms->stackSize--;
                                }
                        } else {
                                puts("Call");
                                ms->pc += 2;
                        }
                        break;
                case 0x1:
                        ms->pc = get12bit(ins);
                        break;
                case 0x2:
                        if(ms->stackSize == ms->stackCapacity){
                                puts("stack overflow!");
                        } else {
                                ms->stack[ms->stackSize++] = ms->pc + 2;
                                ms->pc = get12bit(ins);
                        }
                        break;
                case 0x3:{
                        /* 0x3XNN Vx == N */
                        uint8_t rID;
                        getRegister(ins, &rID);
                        uint8_t n = get8bit(ins);
                        if(ms->registers[rID] == n){
                                ms->pc += 4;
                        } else {
                                ms->pc += 2;
                        }
                        } break;
                case 0x4:{
                        /* 0x4XNN Vx != N */
                        uint8_t rID;
                        getRegister(ins, &rID);
                        int16_t n = get8bit(ins);
                        if(ms->registers[rID] != n)
                                ms->pc += 4;
                        else 
                                ms->pc += 2;
                        } break;
                case 0x5:{
                        /* 0x5XY0 Vx == Vy */
                        /* The instruction must end in a zero */
                        if(ins & 7){
                                puts("invalid");
                        } else {
                                uint8_t rID1, rID2;
                                get2Registers(ins, &rID1, &rID2);
                                if(ms->registers[rID1] == ms->registers[rID2])
                                        ms->pc += 4;
                                else
                                        ms->pc += 2;
                        }
                        break;
                }
                case 0x6:{
                        uint8_t rID;
                        getRegister(ins, &rID);
                        int16_t n = get8bit(ins); 
                        ms->registers[rID] = n;
                        ms->pc += 2;
                        } break;
                case 0x7:{
                        uint8_t rID;
                        getRegister(ins, &rID);
                        int16_t n = get8bit(ins);
                        ms->registers[rID] += n;
                        } break;
                case 0x8:{
                        uint8_t sopc = get4bit(ins);
                        uint8_t rID1; 
                        uint8_t rID2;
                        get2Registers(ins, &rID1, &rID2);
                        switch(sopc){
                                case 0x0:
                                        ms->registers[rID1] = ms->registers[rID2];
                                        break;
                                case 0x1:
                                        ms->registers[rID1] |= ms->registers[rID2];
                                        break;
                                case 0x2:
                                        ms->registers[rID1] &= ms->registers[rID2];
                                        break;
                                case 0x3:
                                        ms->registers[rID1] ^= ms->registers[rID2];
                                        break;
                                case 0x4:
                                        ms->registers[0xF] = ms->registers[rID2] > (255 - ms->registers[rID1]);
                                        ms->registers[rID1] += ms->registers[rID2];
                                        break;
                                case 0x5:
                                        ms->registers[0xF] = ms->registers[rID2] < ms->registers[rID1];
                                        ms->registers[rID1] -= ms->registers[rID2];
                                        break;
                                case 0x6:
                                        ms->registers[0xF] = ms->registers[rID2] & 1;
                                        ms->registers[rID2] = ms->registers[rID2] >> 1;
                                        ms->registers[rID1] = ms->registers[rID2];
                                        break;
                                case 0x7:
                                        ms->registers[0xF] = ms->registers[rID1] < ms->registers[rID2];
                                        ms->registers[rID1] = ms->registers[rID2] - ms->registers[rID1];
                                        break;
                                case 0xE:
                                        ms->registers[0xF] = (ms->registers[rID2] & 0x80) > 0;
                                        ms->registers[rID2] = ms->registers[rID2] << 1;
                                        ms->registers[rID1] = ms->registers[rID2];
                                        break;
                                default:
                                        puts("invalid");
                                        break;

                        }
                }break;
                case 0x9:{
                        uint8_t rID1; 
                        uint8_t rID2;
                        get2Registers(ins, &rID1, &rID2);
                        if(ms->registers[rID1] != ms->registers[rID2])
                                ms->pc += 4;
                        else
                                ms->pc += 2;
                        }break;
                case 0xA:
                        ms->iRegister = get12bit(ins);
                        break;
                case 0xB:
                        ms->pc = ms->registers[0] + get12bit(ins);
                        break;
                case 0xC:{
                        uint8_t rID;
                        getRegister(ins, &rID);
                        ms->registers[rID] = (rand() % 256) & get8bit(ins);
                        }break;
                case 0xD:{
                        uint8_t rID1;
                        uint8_t rID2;
                        uint8_t n = get4bit(ins);
                        get2Registers(ins, &rID1, &rID2);
                        uint8_t x = ms->registers[rID1];
                        uint8_t y = ms->registers[rID2];
                        //printf("n: %u y: %u x: %u\n", n, y, x);
                        /* wrap X to fit in the screan */
                        x = x % 64;
                        /* wrap y to fit in the screen */
                        y = y % 32;
                        //printf("n: %u y: %u x: %u\n", n, y, x);
                        for(size_t i = 0; i < n; i++){
                                uint8_t rData = ms->mem[ms->iRegister + i];
                                /* wrap row if the row is greater > 32 */
                                uint8_t *row = ms->disp[(y + i) % 32];
                                uint8_t vf = 0;
                                /*The first byte that needs updating */
                                uint8_t sCell = x / 8;
                                /* The number needed to change in the next byte */
                                uint8_t overLap = x % 8;
                                /* The second byte that needs updating */
                                uint8_t eCell = (x / 8) + (overLap != 0);

                                //printf("sCell: %u eCell: %u, overLap: %u\n", sCell, eCell, overLap);
                                /* Wrap sprite to the start of the line */
                                if(eCell > 7)
                                        eCell = 0;
                                if(sCell == eCell){
                                        vf = ((row[sCell] & rData) != 0);
                                                row[sCell] ^= rData;
                                } else {
                                        if(row[sCell] & (rData >> overLap))
                                                vf = 1;

                                        row[sCell] ^= rData >> overLap;
                                        if(row[eCell] & (rData << (8 - overLap)))
                                                vf = 1;
                                        row[eCell] ^= rData << (8 - overLap);
                                }
                                ms->registers[0xF] = vf;
                        }
                        /* TODO: render the screen */
                        }break;
                case 0xF:{
                        int16_t sopc = get8bit(ins);
                        switch(sopc){
                                case 0x07:
                                        puts("timer");
                                        break;
                                case 0x0A:
                                        puts("KeyOp");
                                        break;
                        }
                }break;

        }
}
