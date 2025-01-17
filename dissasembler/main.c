#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dissasembler.h"

// const char *TARGET = "../rom_images/4-flags.ch8";

uint8_t *readFile(const char *filename, size_t *ret_size) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t *buffer = (uint8_t *)malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, fileSize, file);
    buffer[fileSize] = '\0'; // Null-terminate the buffer
    *ret_size = fileSize;

    fclose(file);
    return buffer;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    size_t buffSize = 0;
    uint8_t *buffer = readFile(argv[1], &buffSize);
    if (buffer == NULL) {
        printf("read file failed\n");
        return 1;
    }

    Dissasemble(buffer, buffSize);

    free(buffer);
    return 0;
}