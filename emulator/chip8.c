#include <stdlib.h>
#include <stdio.h> /* printf */
#include <string.h> /* memcpy */
#include <stdarg.h> /* va_list, va_start, va_end */

#include "chip8.h"

#define SCREEN_HEIGHT (32)
#define SCREEN_WIDTH (64)
#define SCREEN_SIZE ((SCREEN_HEIGHT) * (SCREEN_WIDTH / CHAR_BIT))
#define CHIP8_RAM_SIZE (4096)
#define VARIABLE_REGISTERS_COUNT (16)

#define FIRST_INSTRUCTION_ADDRESS (0x200)
#define INST_NIBBLES (4)

#define FALSE (0)
#define TRUE (!FALSE)

#define SOLID_BLOCK ("\u2588")
#define BLANK_SPACE ("\u2800")

typedef struct CHIP8State {
    uint64_t screen[SCREEN_HEIGHT];
    uint8_t memory[CHIP8_RAM_SIZE];

    uint16_t pc;
    uint16_t i;
    
    uint8_t v_reg[VARIABLE_REGISTERS_COUNT];

    int is_screen_updated;
} CH8State;

inline static uint16_t Fetch(CH8State *state);
inline static uint16_t ReadInstruction(CH8State *state);
static int Execute(CH8State *state, uint16_t curr_inst);
static void RenderLine(uint64_t line);
static void ExecSprite(CH8State *state, uint8_t x, uint8_t y, uint8_t height);
static void PrintFrameHorLine(void);
static void DebugPrintf(const char *format, ...);

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
    PrintFrameHorLine();
    for (int i = 0; i < SCREEN_HEIGHT; ++i) {
        RenderLine(state->screen[i]);
        printf("\n");
    }
    PrintFrameHorLine();
}

static void PrintFrameHorLine(void) {
    printf(BLANK_SPACE);
    for (int i = 0; i < SCREEN_WIDTH; ++i) {
        printf("_");
    }
    printf("\n");
}

static void RenderLine(uint64_t line) {
    uint64_t bit_runner = 0x1UL << 63;
    printf("|");
    for (int i = 0; i < 64; ++i) {
        bit_runner & line ? printf(SOLID_BLOCK) : printf(BLANK_SPACE);
        bit_runner >>= 1;
    }
    printf("|");
}

void CH8LoadToMemory(CH8State *state, const uint8_t *buff, size_t size) {
    memcpy((state->memory) + FIRST_INSTRUCTION_ADDRESS, buff, size);
}

int CH8Emulate(CH8State *state) {
    uint16_t curr_inst = 0;
    
    DebugPrintf("current instruction: %04X\n", curr_inst);
    while (1) {
        curr_inst = Fetch(state);
        if (Execute(state, curr_inst) != 0) {
            DebugPrintf("Stopping execution\n");
            break;
        }
        if (state->is_screen_updated) {
            CH8Display(state);
            state->is_screen_updated = FALSE;
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
    DebugPrintf("Curr inst: %04X\n", curr_inst);
    uint8_t nib[INST_NIBBLES] = {
        GetNibble(curr_inst, 0),
        GetNibble(curr_inst, 1),
        GetNibble(curr_inst, 2),
        GetNibble(curr_inst, 3),
    }; 

    switch (nib[0])
    {
    case 0x0:
        if (curr_inst == 0x00E0) {
            /* clear screen */
            memset(state->screen, 0, SCREEN_SIZE);
            DebugPrintf("Were on clear screen\n");
        } else if (0x00EE == curr_inst) {
            DebugPrintf("Unimplemented instruciton\n");
        } else {
            DebugPrintf("Unrecognized instruciton\n");
        }    
        break;
    case 0x1:
        state->pc = curr_inst & (uint16_t)0x0FFF;
        if (ReadInstruction(state) == curr_inst) {
            DebugPrintf("Self jump detected\n");
            status = -1;
        }
        break;
    case 0x6:
        state->v_reg[nib[1]] = curr_inst & 0x00FF;
        DebugPrintf("Set reg %01X to %02X\n",  nib[1], curr_inst & 0x00FF);
        break;
    case 0x7: 
        state->v_reg[nib[1]] += curr_inst & 0x00FF;
        break;
    case 0xA:
        DebugPrintf("%04X => i = %03X\n", curr_inst, curr_inst & 0x0FFF);
        state->i = curr_inst & 0x0FFF;
        break;
    case 0xD: /* Ive done it wrong - take a look at specification */
        DebugPrintf("DXYN instruction: %04X\n", curr_inst);
        ExecSprite(state, state->v_reg[nib[1]], state->v_reg[nib[2]], nib[3]);
        break;
    default:
        DebugPrintf("Unrecognized or unimplemented instruction: %04X\n", curr_inst);
        status = -1;
        break;
    }

    return status;
}

static void ExecSprite(CH8State *state, uint8_t x, uint8_t y, uint8_t height) {
    DebugPrintf("Running sprite: x: %d, y: %d, height: %d\n",x, y, height );

    /* implement collision */

    for (int i = 0; i < height; ++i) {
        state->screen[y + i] ^= ((uint64_t)state->memory[state->i + i]) << (63 - x);
    }

    state->is_screen_updated = TRUE;
}

static inline uint16_t GetNibble(uint16_t instruction, int idx) {
    return ((instruction >> (3 - idx) * 4) & 0x000F);
}

static void DebugPrintf(const char *format, ...) {
#ifdef DEBUG
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
#endif /* DEBUG */
}