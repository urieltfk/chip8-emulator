TARGET=emulate.out
DEBUG_TARGET=emulate_debug.out
FILES=chip8.c main.c
INCLUDES=chip8.h
DEBUG_MACRO=DEBUG

$(TARGET): main.c chip8.h chip8.c
	gcc $(FILES) -I $(INCLUDES) -o $@

debug: $(DEBUG_TARGET)

$(DEBUG_TARGET) : $(FILES) $(INCLUDES)
	gcc $(FILES) -I $(INCLUDES) -D$(DEBUG_MACRO) -g -o $@


run :
	./$(TARGET)

clear : 
	rm -rf $(TARGET) *dSYM $(DEBUG_TARGET)
