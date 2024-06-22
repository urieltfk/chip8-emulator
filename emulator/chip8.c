#include <stdlib.h>
#include <stdio.h> /* printf */
#include <string.h> /* memcpy */

#include "chip8.h"

#define FIRST_INSTRUCTION_ADDRESS (0x200)

inline static uint16_t Fetch(CH8State *state);

/* Bitwise utils */


CH8State *CH8Create() {
    CH8State *new_state = (CH8State *)calloc(1, sizeof(CH8State));

    new_state->pc = FIRST_INSTRUCTION_ADDRESS;
    
    return new_state;
}

void CH8Destroy(CH8State *state) {
    free(state);
    state = NULL;
}

void CH8LoadToMemory(CH8State *state, const uint8_t *buff, size_t size) {
    memcpy((state->memory) + FIRST_INSTRUCTION_ADDRESS, buff, size);
}

int CH8Emulate(CH8State *state) {
    uint16_t curr_inst = 0;
    curr_inst = Fetch(state);

    printf("current instruction: %04X\n", curr_inst);

    return 0;
}

inline static uint16_t Fetch(CH8State *state) {
    uint16_t instruction = 0;

    instruction |= ((uint16_t)(state->memory[state->pc]) << CHAR_BIT) | (uint16_t)(state->memory[state->pc + 1]);
    state->pc += 2;

    return instruction;
}


