/* Copyright (C) 2017 David Jowett
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */
#ifndef _CHIP8_H
#define _CHIP8_H
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>

#include "runtime_error.h"

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

void run_instruction(struct mState *ms, uint16_t ins);
struct mState *chip8_init(void);
void chip8_destroy(struct mState **ms);
void chip8_run(struct mState *ms);
void chip8_halt(struct mState *ms);
struct runtime_error *chip8_load_rom(struct mState *ms, char *file);
void chip8_key_event_notify(struct mState *ms, struct keyEvent);
#endif
