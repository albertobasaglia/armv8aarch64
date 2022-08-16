CFLAGS = -ffreestanding -fno-builtin -fno-builtin-function -mcpu=cortex-a53

all: kernel.elf

kernel.elf: _start.o linker.ld start.o _exceptions.o
	aarch64-none-elf-ld -T linker.ld _start.o start.o _exceptions.o -o kernel.elf

_start.o: start.asm
	aarch64-none-elf-as start.asm -o _start.o

_exceptions.o: exceptions.asm
	aarch64-none-elf-as exceptions.asm -o _exceptions.o

start.o: start.c
	aarch64-none-elf-gcc $(CFLAGS) -c start.c -o start.o

clean:
	rm -f _start.o start.o _exceptions.o kernel.elf
