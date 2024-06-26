TARGET=emulate.out
FILES=chip8.c main.c
INCLUDES=chip8.h

$(TARGET): main.c chip8.h chip8.c
	gcc $(FILES) -I $(INCLUDES) -o $(TARGET)

run :
	./$(TARGET)

clear : 
	rm -rf $(TARGET) *dSYM
