#ifndef CHIP8_H
#define CHIP8_H

#include <limits.h> /* CHAR_BIT */
#include <stdint.h> /* uint8_t */
#include <stdlib.h> /* size_t */

typedef struct CHIP8State CH8State;

CH8State *CH8Create();
void CH8Destroy(CH8State *state);

const uint8_t *CH8GetScreen(CH8State *state);
void CH8LoadToMemory(CH8State *state, const uint8_t *buff, size_t size);
int CH8Emulate(CH8State *state);

#endif //CHIP8_H