#include <stdio.h>

#include "chip8.h"

int main(void) {
    printf("Welcome to chip8 emulator\n");

    CH8State *system = CH8Create();
    if (system == NULL) {
        printf("Failed to init system, closing...\n");
        return -1;
    }

    CH8Emulate(system);

    CH8Destroy(system);
    system = NULL;

    return 0;
}

// gcc main.c chip8.c -I chip8.h