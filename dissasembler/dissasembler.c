#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "dissasembler.h"

void Dissasemble(const uint8_t *binary, size_t size) {
    assert(binary != NULL);

    for (size_t i = 0; i < size; i+=2)
    {
        if (i % 10 == 0) {
            printf("\n");
        }
        printf("%02X%02X ", binary[i], binary[i + 1]);
    }
}