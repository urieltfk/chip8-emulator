#include <stdio.h>

#include "chip8.h"

uint8_t *readFile(const char *filename, size_t *ret_size);
const uint8_t print_E_to_screen[] = {
    0xA2, 0x0A, 0x60, 0x0A, 0x61, 0x05,
    0xD0, 0x17, 0x12, 0x08, 0x7C, 0x40,
    0x40, 0x7C, 0x40, 0x40, 0x7C
};

int main(void) {
    printf("Welcome to chip8 emulator\n");

    size_t read_size = 0;
    uint8_t *img_buff = readFile("../rom_images/ibm_logo.ch8", &read_size);
    if (img_buff == NULL) {
        printf("Failed to read executable\n");
        return -1;
    }

    CH8State *system = CH8Create();
    if (system == NULL) {
        printf("Failed to init system, closing...\n");
        return -1;
    }

    /* CH8LoadToMemory(system, img_buff, read_size); */

    CH8LoadToMemory(system, print_E_to_screen, sizeof(print_E_to_screen));
    free(img_buff);
    img_buff = NULL;

    CH8Emulate(system);

    CH8Destroy(system);
    system = NULL;

    return 0;
}

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

// gcc main.c chip8.c -I chip8.h