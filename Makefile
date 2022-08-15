CFLAGS = -ffreestanding -g -fno-builtin -fno-builtin-function -mcpu=cortex-a53

all: kernel.elf

kernel.elf: _start.o linker.ld start.o
	aarch64-none-elf-ld -T linker.ld _start.o start.o -o kernel.elf

_start.o: start.asm
	aarch64-none-elf-as start.asm -o _start.o

start.o: start.c
	aarch64-none-elf-gcc $(CFLAGS) -c start.c -o start.o

clean:
	rm -f _start.o start.o kernel.elf
