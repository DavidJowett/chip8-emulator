#include <stdlib.h>
#include <stdio.h>

#include "chip8.h"

void usage(int argc, char *argv[]){
        printf("%s <ROM>\n", argv[0]);
}


int main(int argc, char *argv[]){
        struct mState *chip;
        struct runtime_error *re;
        
        srand(time(NULL));

        if(argc < 2){
                usage(argc, argv);
                return 0;
        }
        
        chip = chip8_init();

        re = chip8_load_rom(chip, argv[1]);
        if(re != NULL){
                printf("%s\n", re->msg);
                return -1;
        }

        chip8_run(chip);

        chip8_wait_for_ui_stop(chip);
      
        chip8_destroy(&chip);
        return 0;
}
