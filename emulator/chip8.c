#include <stdlib.h>

#include "chip8.h"

CH8State *CH8Create() {
    CH8State *new_state = (CH8State *)calloc(1, sizeof(CH8State));
    
    return new_state;
}

void CH8Destroy(CH8State *state) {
    free(state);
    state = NULL;
}

int CH8Emulate(CH8State *system_state) {


    return 0;
}


