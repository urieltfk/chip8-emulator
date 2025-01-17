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
#define INTRA_CYCLE_DELAY (10) // 100000
#define CALL_STACK_SIZE (16)

#define FIRST_INSTRUCTION_ADDRESS (0x200)
#define INST_NIBBLES (4)
#define EMPTY_STACK (-1)

#define FALSE (0)
#define TRUE (!FALSE)
#define BASE10 (10)

#define SOLID_BLOCK ("\u2588")
#define BLANK_SPACE ("\u2800")

/* fucntion like */
#define GET_REG(state, reg) ((state)->v_reg[reg])

enum ch8_status_t {
    SUCCESS = 0,
    INF_LOOP,
    OP_CODE_ERR,
};

typedef struct CHIP8State {
    uint64_t screen[SCREEN_HEIGHT];
    uint8_t memory[CHIP8_RAM_SIZE];

    uint16_t pc;
    uint16_t i;
    
    uint8_t v_reg[VARIABLE_REGISTERS_COUNT];
    uint16_t call_stack[CALL_STACK_SIZE];
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
static inline int HasOverflow8Add(uint8_t a, uint8_t b);

/* instructions */
static inline int Inst0xFx33(CH8State *state, uint8_t x);
static inline int Inst0xFx55(CH8State *state, uint8_t x);
static inline int Inst0xFx65(CH8State *state, uint8_t x);
static inline int Inst0xFx1E(CH8State *state, uint8_t x);

/* Bitwise utils */
static inline uint16_t GetNibble(uint16_t instruction, int idx);
int HasCollision(uint64_t curr_line, uint64_t pix_to_xor);
uint64_t RotateRowLeft(uint64_t row, size_t n_positions);
static inline uint8_t GetLSByte(uint16_t inst);
static inline uint8_t GetMSByte(uint16_t inst);
static inline uint8_t ReverseByte(uint8_t b);

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
    uint64_t bit_runner = 0x1UL;
    printf("|");
    for (int i = 0; i < 64; ++i) {
        bit_runner & line ? printf(SOLID_BLOCK) : printf(BLANK_SPACE);
        bit_runner <<= 1;
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
    uint8_t MSbyte = (curr_inst & 0xFF00) >> CHAR_BIT;
    uint8_t LSbyte = (curr_inst & 0x00FF);
    uint8_t flag_reg_value = 0;
    /* Vx Vy pointers? */

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
    case 0x8: 
        DebugPrintf("Running 8xyN instruction: %04X\n", curr_inst);
        switch (nib[3])
        {
        case 0x0:
            DebugPrintf("V[%d] = V[%d]", nib[1], nib[2]);
            state->v_reg[nib[1]] = state->v_reg[nib[2]];
            break;
        case 0x1:
            DebugPrintf("V[%d] |= V[%d]", nib[1], nib[2]); 
            state->v_reg[nib[1]] |= state->v_reg[nib[2]];
            break;
        case 0x2:
            DebugPrintf("V[%d] &= V[%d]", nib[1], nib[2]); 
            state->v_reg[nib[1]] &= state->v_reg[nib[2]];
            break;
        case 0x3:
            DebugPrintf("V[%d] ^= V[%d]", nib[1], nib[2]); 
            state->v_reg[nib[1]] ^= state->v_reg[nib[2]];
            break;
        case 0x4:
            flag_reg_value = !!HasOverflow8Add(state->v_reg[nib[1]], state->v_reg[nib[2]]);
            state->v_reg[nib[1]] += state->v_reg[nib[2]];
            state->v_reg[0xF] = flag_reg_value;
            break;
        case 0x5:
            flag_reg_value = state->v_reg[nib[1]] >= state->v_reg[nib[2]];
            state->v_reg[nib[1]] -= state->v_reg[nib[2]];
            state->v_reg[0xF] = flag_reg_value;
            break;
        case 0x6:
            flag_reg_value = state->v_reg[nib[1]] & 0x1;
            state->v_reg[nib[1]] >>= 1;
            state->v_reg[0xF] = flag_reg_value;
            break;
        case 0x7:
            flag_reg_value = state->v_reg[nib[2]] >= state->v_reg[nib[1]];
            state->v_reg[nib[1]] = state->v_reg[nib[2]] - state->v_reg[nib[1]];
            state->v_reg[0xF] = flag_reg_value;
            break;
        case 0xE:
            flag_reg_value = !!(state->v_reg[nib[1]] & ((uint8_t)0x1 << 7));
            state->v_reg[nib[1]] <<= 1;
            state->v_reg[0xF] = flag_reg_value;
            break;
        default:
            DebugPrintf("Unrecognized or unimplemented instruction: %04X\n", curr_inst);
            status = OP_CODE_ERR;
            break;
        }
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
    case 0xB:
        state->pc = curr_inst & 0x0FFF + state->v_reg[0];
        break;
    case 0xC:
        state->v_reg[nib[1]] = rand() % 0xFF & curr_inst & 0x00FF;
        break;
    case 0xD:
        DebugPrintf("DXYN instruction: %04X\n", curr_inst);
        ExecSprite(state, state->v_reg[nib[1]], state->v_reg[nib[2]], nib[3]);
        break;
    case 0xF:
        switch (LSbyte)
        {
        case 0x33: 
            status = Inst0xFx33(state, nib[1]);
            break;
        case 0x55:
            status = Inst0xFx55(state, nib[1]);
            break;
        case 0x65:
            status = Inst0xFx65(state, nib[1]); 
            break;
        case 0x1E:
            status = Inst0xFx1E(state, nib[1]);
            break;
        default:
            status = OP_CODE_ERR;
            break;
        }
    break;
    default:
        DebugPrintf("Unrecognized or unimplemented instruction: %04X\n", curr_inst);
        status = OP_CODE_ERR;
        break;
    }

    return status;
}

static void ExecSprite(CH8State *state, uint8_t x, uint8_t y, uint8_t height) {
    DebugPrintf("Running sprite: x: %d, y: %d, height: %d\n", x, y, height );
    int has_collision = 0;

    for (int i = 0; i < height; ++i) {
        uint8_t byte_to_draw = ReverseByte(state->memory[state->i + i]);
        uint64_t line_to_xor = RotateRowLeft(byte_to_draw, x);
        has_collision |= !!(state->screen[y + i] & line_to_xor);
        state->screen[y + i] ^= line_to_xor;
    }

    state->is_screen_updated = TRUE;
    state->v_reg[0xF] = has_collision;
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

static inline uint8_t ReverseByte(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

static void DelayByMS(size_t ms) {
    clock_t target_time = clock() + ms;
    
    while (clock() < target_time)
    {
    }
}

static inline int HasOverflow8Add(uint8_t a, uint8_t b) {
    return (a > (UCHAR_MAX - b));
}

static inline int Inst0xFx65(CH8State *state, uint8_t x) {
    assert(x < (VARIABLE_REGISTERS_COUNT - 1));
    assert(state);
    
    for (int i = 0; i <= x; ++i) {
        state->v_reg[i] = state->memory[state->i + i];
    }

    return 0;
}

static inline int Inst0xFx55(CH8State *state, uint8_t x) {
    assert(x < (VARIABLE_REGISTERS_COUNT - 1));
    assert(state);

    for (int i = 0; i <= x; ++i) {
        state->memory[state->i + i] = state->v_reg[i];
    }

    return 0;
}

static inline int Inst0xFx33(CH8State *state, uint8_t x) {
    assert(x < (VARIABLE_REGISTERS_COUNT - 1));
    assert(state);
    
    uint8_t x_val = state->v_reg[x];
    for (int i = 2; i >= 0; --i) {
        uint8_t curr_digit = x_val % BASE10;
        x_val /= BASE10;
        state->memory[state->i + i] = curr_digit;
    }

    return 0;
}

static inline int Inst0xFx1E(CH8State *state, uint8_t x) {
    state->i += state->v_reg[x];

    return 0;
}