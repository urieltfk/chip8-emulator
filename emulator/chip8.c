#include <stdlib.h>
#include <stdio.h> /* printf */
#include <string.h> /* memcpy */
#include <stdarg.h> /* va_list, va_start, va_end */
#include <time.h> /* clock_t */
#include <assert.h> /* assert */

#include "chip8.h"

#define SCREEN_HEIGHT (32)
#define SCREEN_WIDTH (64)
#define SCREEN_SIZE ((SCREEN_HEIGHT) * (SCREEN_WIDTH / CHAR_BIT))
#define CHIP8_RAM_SIZE (4096)
#define VARIABLE_REGISTERS_COUNT (16)
#define INTRA_CYCLE_DELAY (100000)
#define CALL_STACK_SIZE (16)

#define FIRST_INSTRUCTION_ADDRESS (0x200)
#define INST_NIBBLES (4)
#define EMPTY_STACK (-1)

#define FALSE (0)
#define TRUE (!FALSE)

#define SOLID_BLOCK ("\u2588")
#define BLANK_SPACE ("\u2800")

enum ch8_status_t {
    SUCCESS = 0,
    INF_LOOP,
};

typedef struct CHIP8State {
    uint64_t screen[SCREEN_HEIGHT];
    uint8_t memory[CHIP8_RAM_SIZE];

    uint16_t pc;
    uint16_t i;
    
    uint8_t v_reg[VARIABLE_REGISTERS_COUNT];
    uint8_t call_stack[CALL_STACK_SIZE];
    int stack_top;

    int is_screen_updated;
} CH8State;

inline static uint16_t Fetch(CH8State *state);
inline static uint16_t ReadInstruction(CH8State *state);
static int Execute(CH8State *state, uint16_t curr_inst);
static void RenderLine(uint64_t line);
static void ExecSprite(CH8State *state, uint8_t x, uint8_t y, uint8_t height);
static void PrintFrameHorLine(void);
static void DebugPrintf(const char *format, ...);
static void DelayByMS(size_t ms);

/* Bitwise utils */
static inline uint16_t GetNibble(uint16_t instruction, int idx);
int HasCollision(uint64_t curr_line, uint64_t pix_to_xor);
uint64_t RotateRowLeft(uint64_t row, size_t n_positions);
static inline uint8_t GetLSByte(uint16_t inst);
static inline uint8_t GetMSByte(uint16_t inst);

CH8State *CH8Create() {
    CH8State *new_state = (CH8State *)calloc(1, sizeof(CH8State));

    new_state->pc = FIRST_INSTRUCTION_ADDRESS;
    new_state->stack_top = EMPTY_STACK;
    
    return new_state;
}

void CH8Destroy(CH8State *state) {
    free(state);
    state = NULL;
}

void CH8Display(CH8State *state) {
#ifndef DEBUG
    system("clear");
#endif
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
    int status = 0;
    
    DebugPrintf("current instruction: %04X\n", curr_inst);
    while (1) {
        curr_inst = Fetch(state);

        status = Execute(state, curr_inst);
        if (status != SUCCESS) {
            DebugPrintf("Stopping execution, status: %d\n", status);
            break;
        }

        DelayByMS(INTRA_CYCLE_DELAY);
        if (state->is_screen_updated) {
            CH8Display(state);
            state->is_screen_updated = FALSE;
        }
    }
    

    return status;
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
            assert(state->stack_top > EMPTY_STACK);
            state->pc = state->call_stack[state->stack_top];
            state->stack_top--;
        } else {
            status = -1;
            DebugPrintf("Unrecognized instruciton\n");
        }    
        break;
    case 0x1:
        state->pc = curr_inst & (uint16_t)0x0FFF;
        if (ReadInstruction(state) == curr_inst) {
            printf("Self jump detected\n");
            status = INF_LOOP;
        }
        break;
    case 0x2:
        state->stack_top++;
        assert(state->stack_top > EMPTY_STACK && state->stack_top < CALL_STACK_SIZE);
        state->call_stack[state->stack_top] = state->pc;
        state->pc = curr_inst & 0x0FFF;
        break;
    case 0x3:
        if (state->v_reg[nib[1]] == GetLSByte(curr_inst)) {
            state->pc += 2;
        }
        break;
    case 0x4: 
        if (state->v_reg[nib[1]] != GetLSByte(curr_inst)) {
            state->pc += 2;
        }
        break;
    case 0x5:
        if (state->v_reg[nib[1]] == state->v_reg[nib[2]]) {
            state->pc += 2;
        }
        break;
    case 0x6:
        state->v_reg[nib[1]] = curr_inst & 0x00FF;
        DebugPrintf("Set reg %01X to %02X\n",  nib[1], GetLSByte(curr_inst));
        break;
    case 0x7: 
        state->v_reg[nib[1]] += curr_inst & 0x00FF;
        break;
    case 0x9: 
        if (state->v_reg[nib[1]] != state->v_reg[nib[2]]) {
            state->pc += 2;
        }
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
    int has_collision = 0;

    for (int i = 0; i < height; ++i) {
        uint64_t line_to_xor = RotateRowLeft(state->memory[state->i + i] , 63 - x - 8); /* Theres a bug here, this -8 shouldnt be there..., find why it works correctly with it and fix. */
        has_collision = has_collision | HasCollision(state->screen[y + i], line_to_xor);
        state->screen[y + i] ^= line_to_xor;
    }

    state->is_screen_updated = TRUE;
    state->v_reg[0xF] = !!has_collision;
}

int HasCollision(uint64_t curr_line, uint64_t pix_to_xor) {
    return curr_line & pix_to_xor;
}

uint64_t RotateRowLeft(uint64_t row, size_t n_positions) {
    return (row << n_positions) | (row >> (63 - n_positions));
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

static inline uint8_t GetLSByte(uint16_t inst) {
    return inst & 0x00FF;
}

static inline uint8_t GetMSByte(uint16_t inst) {
    return (inst & 0xFF00) >> CHAR_BIT;
}

static void DelayByMS(size_t ms) {
    clock_t target_time = clock() + ms;
    
    while (clock() < target_time)
    {
    }
}