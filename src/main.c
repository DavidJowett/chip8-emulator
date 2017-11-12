#include <stdlib.h>
#include "chip8.h"


int main(int argc, char *argv[]){
        struct mState *ms = malloc(sizeof(struct mState));

        runInstruction(ms, 0x1003);
        
        return 0;
}
