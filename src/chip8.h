#ifndef _CHIP8_H
#define _CHIP8_H
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>

enum keyEventType{Pressed, Released};

struct keyEvent {
        enum keyEventType type;
        uint8_t key;
};

struct mState {
        uint64_t count;
        uint8_t registers[16];
        uint16_t iRegister;
        int16_t pc;
        int16_t *stack;
        uint8_t disp[32][8];
        size_t stackSize;
        size_t stackCapacity;
        uint8_t mem[4096];

        uint8_t keys[16];
        struct keyEvent lastEvent;

        /* Timers */
        uint8_t dTimer;
        uint8_t sTimer;

        /* Mutexs */
        pthread_mutex_t timerMutex;
        pthread_mutex_t keyMutex;
        /* The timer thread */
        pthread_t tThread;
        /* The execution thread */
        pthread_t eThread;
        
        /* shared running flag */
        uint8_t running;

        /* The incoming key press pthread_cond_t */
        pthread_cond_t incomingKeyEvent;
};

struct runtime_error {
        char *msg;
};

void run_instruction(struct mState *ms, uint16_t ins);
struct mState *chip8_init(void);
void chip8_destroy(struct mState **ms);
void chip8_run(struct mState *ms);
void chip8_halt(struct mState *ms);
void chip8_key_event_notify(struct mState *ms, struct keyEvent);
#endif
