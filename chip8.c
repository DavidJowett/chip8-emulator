#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "chip8.h"


/* The font table. 4x5 charactrs for 0-9 A-F */
#define FONT_LEN 80
static const uint8_t font[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0,
        0x20, 0x60, 0x20, 0x20, 0x70,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x10, 0xF0,
        0x90, 0x90, 0xF0, 0x10, 0x10,
        0xF0, 0x80, 0xF0, 0x10, 0xF0,
        0xF0, 0x80, 0xF0, 0x90, 0xF0,
        0xF0, 0x10, 0x20, 0x40, 0x40,
        0xF0, 0x90, 0xF0, 0x90, 0xF0,
        0xF0, 0x90, 0xF0, 0x10, 0xF0,
        0xF0, 0x90, 0xF0, 0x90, 0x90,
        0xE0, 0x90, 0xE0, 0x90, 0xE0,
        0xF0, 0x80, 0x80, 0x80, 0xF0,
        0xE0, 0x90, 0x90, 0x90, 0xE0,
        0xF0, 0x80, 0xF0, 0x80, 0xF0,
        0xF0, 0x80, 0xF0, 0x80, 0x80};

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

static void *timerThread(void *data){
        struct mState *ms = (struct mState *) data;
        struct timespec ts, ts2;
        ts.tv_nsec = 16666666L;
        ts.tv_sec = 0;
        while(ms->running){
                pthread_mutex_lock(&ms->timerMutex);
                if(ms->dTimer != 0) ms->dTimer--;
                if(ms->sTimer != 0) ms->sTimer--;
                pthread_mutex_unlock(&ms->timerMutex);
                nanosleep(&ts, &ts2);
        }
        pthread_exit(NULL);
}

static void *executionThread(void * data){
        struct mState *ms = (struct mState *) data;
        ms->count = 0;
        while(ms->running){
                uint16_t ins;
                uint8_t lsins, msins;
                msins = ms->mem[ms->pc];
                lsins = ms->mem[ms->pc + 1];
                ins = ((uint16_t) (msins)) << 8;
                ins |= lsins;
                run_instruction(ms, ins);
                ms->count++;
                if(ms->pc > 4095){
                        puts("PC > memory size");
                        pthread_exit(NULL);
                }
        }
        pthread_exit(NULL);
}

void chip8_key_event_notify(struct mState *ms, struct keyEvent ke){
        if(ke.key > 0xF){
                puts("Invalid keycode!");
                return;
        }
        pthread_mutex_lock(&ms->keyMutex);
        ms->lastEvent = ke;
        if(ke.type == Pressed){
                ms->keys[ke.key] = 1;
        } else {
                ms->keys[ke.key] = 0;
        }
        pthread_cond_signal(&ms->incomingKeyEvent);
        pthread_mutex_unlock(&ms->keyMutex);

}

struct mState *chip8_init(void){
        struct mState *ms = malloc(sizeof(struct mState));
        if(ms == NULL) return NULL;
        ms->stack = calloc(48, sizeof(int16_t));
        if(ms->stack == NULL) goto mStateInitFail;
        ms->stackSize = 0;
        ms->stackCapacity = 48;
        ms->pc = 0;
        ms->sTimer = 0;
        ms->dTimer = 0;
        ms->iRegister = 0;
        for(int i = 0; i < 16; i++)
                ms->registers[i] = 0;
        clear_display(ms);

        /* Zero the key state */
        for(size_t i = 0; i < 16; i++){
                ms->keys[i] = 0;
        }



        /* Setup up the font
         * http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#2.4 */
        for(size_t i = 0; i < FONT_LEN; i++)
                ms->mem[i] = font[i];

        return ms;

mStateInitFail:
        free(ms);
        return NULL;
}

void chip8_destroy(struct mState **ms){
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
                                printf("Invalid %x %x\n", ins, ms->pc);
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
                        ms->pc += 2;
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
                                        printf("Invalid %x %x\n", ins, ms->pc);
                                        puts("invalid2");
                                        break;

                        }
                        ms->pc += 2;
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
                        ms->pc += 2;
                        break;
                case 0xB:
                        ms->pc = ms->registers[0] + get12bit(ins);
                        break;
                case 0xC:{
                        uint8_t rID;
                        getRegister(ins, &rID);
                        ms->registers[rID] = (rand() % 256) & get8bit(ins);
                        ms->pc += 2;
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
                        ms->pc += 2;
                        /* TODO: render the screen */
                        }break;
                case 0xE:{
                        int16_t sopc = get8bit(ins);
                        uint8_t rID;
                        getRegister(ins, &rID);
                        switch(sopc){
                                case 0x9E:
                                        if(ms->keys[ms->registers[rID]])
                                                ms->pc += 4;
                                        else
                                                ms->pc += 2;
                                        break;
                                case 0xA1:
                                        if(!ms->keys[ms->registers[rID]])
                                                ms->pc += 4;
                                        else
                                                ms->pc += 2;
                                        break;
                        }

                        } break;
                case 0xF:{
                        int16_t sopc = get8bit(ins);
                        uint8_t rID;
                        getRegister(ins, &rID);
                        switch(sopc){
                                case 0x07:
                                        pthread_mutex_lock(&ms->timerMutex);
                                        ms->registers[rID] = ms->dTimer;
                                        pthread_mutex_unlock(&ms->timerMutex);
                                        break;
                                case 0x0A:{
                                        while(1){
                                                pthread_mutex_lock(&ms->keyMutex);
                                                pthread_cond_wait(&ms->incomingKeyEvent, &ms->keyMutex);
                                                if(ms->lastEvent.type == Pressed){
                                                        ms->registers[rID] = ms->lastEvent.key;
                                                        break;
                                                }
                                                pthread_mutex_unlock(&ms->keyMutex);
                                        }
                                        pthread_mutex_unlock(&ms->keyMutex);
                                        }break;
                                case 0x15:
                                        pthread_mutex_lock(&ms->timerMutex);
                                        ms->dTimer = ms->registers[rID];
                                        pthread_mutex_unlock(&ms->timerMutex);
                                        break;
                                case 0x18:
                                        pthread_mutex_lock(&ms->timerMutex);
                                        ms->sTimer = ms->registers[rID];
                                        pthread_mutex_unlock(&ms->timerMutex);
                                        break;
                                case 0x1E:
                                        ms->iRegister += ms->registers[rID];
                                        break;
                                case 0x29:
                                        ms->iRegister = ms->registers[rID] * 5;
                                        break;
                                case 0x33:
                                        ms->mem[ms->iRegister] = ms->registers[rID] / 100;
                                        ms->mem[ms->iRegister + 1] = (ms->registers[rID] % 100) / 10;
                                        ms->mem[ms->iRegister + 2] = (ms->registers[rID] % 10);
                                        break;
                                case 0x55:
                                        for(size_t i = 0; i <= rID; i++)
                                                ms->mem[ms->iRegister++] = ms->registers[i];
                                        break;
                                case 0x65:
                                        for(size_t i = 0; i <= rID; i++)
                                                ms->registers[i] = ms->mem[ms->iRegister++];
                                        break;
                        }
                        ms->pc += 2;
                        }break;

        }
}

void chip8_run(struct mState *ms){
        /* set the state to running */
        ms->running = 1;

        /* Setup mutexs */
        pthread_mutex_init(&ms->timerMutex, NULL);
        pthread_mutex_init(&ms->keyMutex, NULL);

        /* Setup the conditon variables */
        pthread_cond_init(&ms->incomingKeyEvent, NULL);

        pthread_create(&ms->tThread, NULL, timerThread, ((void *) ms));
        pthread_create(&ms->eThread, NULL, executionThread, ((void *) ms));
}

void chip8_halt(struct mState *ms){
        /* stop the threads */
        ms->running = 0;

        /* wait for the threads to terminate */
        pthread_join(ms->eThread, NULL);
        pthread_join(ms->tThread, NULL);

        /* TODO stop the notification of key events */
        
        /* destroy the condition varibales */
        pthread_cond_destroy(&ms->incomingKeyEvent);

        /* destroy the mutexs */
        pthread_mutex_destroy(&ms->timerMutex);
        pthread_mutex_destroy(&ms->keyMutex);

}
