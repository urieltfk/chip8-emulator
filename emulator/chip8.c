#include <stdlib.h>
#include <stdio.h> /* printf */
#include <string.h> /* memcpy */

#include "chip8.h"

#define FIRST_INSTRUCTION_ADDRESS (0x200)

inline static uint16_t Fetch(CH8State *state);
inline static uint16_t ReadInstruction(CH8State *state);
static int Execute(CH8State *state, uint16_t curr_inst);
static void RenderLine(uint8_t *line, size_t line_size);
static void ExecSprite(CH8State *state, uint8_t x, uint8_t y, uint8_t height);

/* Bitwise utils */
static inline uint16_t GetNibble(uint16_t instruction, int idx);


CH8State *CH8Create() {
    CH8State *new_state = (CH8State *)calloc(1, sizeof(CH8State));

    new_state->pc = FIRST_INSTRUCTION_ADDRESS;
    
    return new_state;
}

void CH8Destroy(CH8State *state) {
    free(state);
    state = NULL;
}

void CH8Display(CH8State *state) {
    system("clear");
    for (int i = 0; i < SCREEN_HEIGHT; ++i) {
        RenderLine(state->screen[i], SCREEN_WIDTH / CHAR_BIT);
        printf("\n");
    }
}

static void RenderLine(uint8_t *line, size_t line_size) {
    for (int i = 0; i < line_size; ++i) {
        uint8_t curr_byte = line[i];
        for (int j = 0; j < CHAR_BIT; j++) {
            curr_byte & 0x10 ? printf("X") : printf(".");
            curr_byte <<= 1;
        }
    }
}

void CH8LoadToMemory(CH8State *state, const uint8_t *buff, size_t size) {
    memcpy((state->memory) + FIRST_INSTRUCTION_ADDRESS, buff, size);
}

int CH8Emulate(CH8State *state) {
    uint16_t curr_inst = 0;
    
    printf("current instruction: %04X\n", curr_inst);
    while (1) {
        curr_inst = Fetch(state);
        if (Execute(state, curr_inst) != 0) {
            printf("Stopping execution\n");
            break;
        }
    }
    

    return 0;
}

inline static uint16_t Fetch(CH8State *state) {
    uint16_t instruction = 0;

    instruction |= ((uint16_t)(state->memory[state->pc]) << CHAR_BIT) | (uint16_t)(state->memory[state->pc + 1]);
    state->pc += 2;

    return instruction;
}

inline static uint16_t ReadInstruction(CH8State *state) {
    return ((uint16_t)(state->memory[state->pc]) << CHAR_BIT) | (uint16_t)(state->memory[state->pc + 1]);
}

static int Execute(CH8State *state, uint16_t curr_inst) {    
    int status = 0;
    printf("Curr inst: %04X\n", curr_inst);

    switch (GetNibble(curr_inst, 0))
    {
    case 0x0:
        if (curr_inst == 0x00E0) {
            /* clear screen */
            memset(state->screen, 0, SCREEN_SIZE);
            printf("Were on clear screen\n");
        } else if (0x00EE == curr_inst) {
            /* uniplemented */
        } else {
            printf("Unrecognized instruciton\n");
        }    
        break;
    case 0x1:
        /* check for self jump - see docs */
        state->pc = curr_inst & (uint16_t)0x0FFF;
        if (ReadInstruction(state) == curr_inst) {
            printf("Self jump detected\n");
            status = -1;
        }
        break;
    case 0x6:
        state->v_reg[GetNibble(curr_inst, 1)] = curr_inst & 0x00FF;
        printf("Set reg %01X to %02X\n",  GetNibble(curr_inst, 1), curr_inst & 0x00FF);
        break;
    case 0x7: 
        state->v_reg[GetNibble(curr_inst, 1)] = curr_inst & 0x00FF;
        break;
    case 0xA:
        printf("%04X => i = %03X\n", curr_inst, curr_inst & 0x0FFF);
        state->i = curr_inst & 0x0FFF;
        break;
    case 0xD: /* Ive done it wrong - take a look at specification */
        printf("DXYN instruction: %04X\n", curr_inst);
        ExecSprite(state, GetNibble(curr_inst, 1), GetNibble(curr_inst, 2), GetNibble(curr_inst, 3));
        CH8Display(state);
        break;
    default:
        printf("Unrecognized or unimplemented instruction: %04X\n", curr_inst);
        status = -1;
        break;
    }

    return status;
}

static void ExecSprite(CH8State *state, uint8_t x, uint8_t y, uint8_t height) {
    printf("Running sprite: x: %d, y: %d, height: %d\n",x, y, height );
    int has_collision = 0;

    for (int i = 0; i < height; ++i) {
        has_collision |= !!state->screen[y + i][x];
        state->screen[y + i][x] ^= 0xFF;
    }

    state->v_reg[0xF] = !!has_collision;
}

static inline uint16_t GetNibble(uint16_t instruction, int idx) {
    return ((instruction >> (3 - idx) * 4) & 0x000F);
}