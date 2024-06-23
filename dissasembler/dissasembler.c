#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "dissasembler.h"

#define CHIP8_MEM_OFFSET (0x200)

void Dissasemble(const uint8_t *binary, size_t size) {
    assert(binary != NULL);

    for (size_t i = 0; i < size; i+=2)
    {
        if (i % 10 == 0) {
            printf("\n");
            printf("%05d | %05X : ", i + CHIP8_MEM_OFFSET, i + CHIP8_MEM_OFFSET);
        }
        printf("%02X%02X ", binary[i], binary[i + 1]);
    }
}