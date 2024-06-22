#include <stdlib.h>
#include <stdio.h>

#include "chip8.h"

inline static uint16_t Fetch(CH8State *state);

CH8State *CH8Create() {
    CH8State *new_state = (CH8State *)calloc(1, sizeof(CH8State));

    new_state->memory[0] = 0xE0;
    new_state->memory[1] = 0x00;
    
    return new_state;
}

void CH8Destroy(CH8State *state) {
    free(state);
    state = NULL;
}

int CH8Emulate(CH8State *state) {
    uint16_t curr_inst = 0;
    curr_inst = Fetch(state);

    printf("current instruction: %04X\n", curr_inst);

    return 0;
}

inline static uint16_t Fetch(CH8State *state) {
    uint16_t instruction = 0;

    instruction |= ((uint16_t)(state->memory[state->pc])) | (uint16_t)(state->memory[state->pc + 1]  << CHAR_BIT);
    state->pc += 2;

    return instruction;
}


