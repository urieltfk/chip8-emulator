#ifndef CHIP8_H
#define CHIP8_H

#include <limits.h> /* CHAR_BIT */
#include <stdint.h> /* uint8_t */

#define SCREEN_HEIGHT (32)
#define SCREEN_WIDTH (64)
#define CHIP8_RAM_SIZE (4096)

typedef struct CHIP8State {
    uint8_t screen[SCREEN_HEIGHT / CHAR_BIT][SCREEN_WIDTH / CHAR_BIT];
    uint8_t memory[CHIP8_RAM_SIZE];

    uint16_t pc;
    uint16_t i;
    
    
} CH8State;

CH8State *CH8Create();
void CH8Destroy(CH8State *state);

const uint8_t *CH8GetScreen(CH8State *state);
int CH8Emulate(CH8State *state);

#endif //CHIP8_H