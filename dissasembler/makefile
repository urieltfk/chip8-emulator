EXECUTABLE = dissassemble.out

$(EXECUTABLE): main.c dissasembler.h dissasembler.c 
	gcc main.c dissasembler.c -I dissasembler.h -o $(EXECUTABLE)

clear: 
	rm $(EXECUTABLE)
