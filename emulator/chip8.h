#ifndef CHIP8_H
#define CHIP8_H

#include <limits.h> /* CHAR_BIT */
#include <stdint.h> /* uint8_t */
#include <stdlib.h> /* size_t */

#define SCREEN_HEIGHT (32)
#define SCREEN_WIDTH (64)
#define SCREEN_SIZE ((SCREEN_HEIGHT) * (SCREEN_WIDTH / CHAR_BIT))
#define CHIP8_RAM_SIZE (4096)
#define VARIABLE_REGISTERS_COUNT (16)

typedef struct CHIP8State {
    uint64_t screen[SCREEN_HEIGHT];
    uint8_t memory[CHIP8_RAM_SIZE];

    uint16_t pc;
    uint16_t i;
    
    uint8_t v_reg[VARIABLE_REGISTERS_COUNT];
    
} CH8State;

CH8State *CH8Create();
void CH8Destroy(CH8State *state);

const uint8_t *CH8GetScreen(CH8State *state);
void CH8LoadToMemory(CH8State *state, const uint8_t *buff, size_t size);
int CH8Emulate(CH8State *state);

#endif //CHIP8_H