# Docs
This file contains notes regarding the behvior of chip8 system I found out along the way.

- Self jump - In order to stop a program, some programs used a mechanisem of self jumping where 
an 0x1NNN op-code would jump to itself creating an infinite loop, This loop would be detected by the system and cause the program to finish.
So infinite loops may be a common thing in CHIP8 systems.

- Chip8 ROMS are big endian.