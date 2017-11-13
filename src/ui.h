#ifndef _SRC_UI_H
#define _SRC_UI_H
#include <pthread.h>
#include <stdint.h>

#include "chip8.h"
#include "state.h"

struct ui {
        /* chip8 emulator core */
        struct mState *chip;

        uint8_t chip8Disp[32][8];
        uint8_t newData;
        enum running_state state;

        /* threading variables */
        pthread_mutex_t dispMutex;
        pthread_t tid;

        
        /* Gets notified when the UI starts or stops */
        pthread_cond_t uiStateChange;
        pthread_mutex_t stateMutex;
};

struct ui *ui_init(void);
void ui_destroy(struct ui **u);
int ui_set_chip8_display(struct ui *u, uint8_t chip8Disp[32][8]);
void ui_run(struct ui *u);
void ui_halt(struct ui *u);

#endif
