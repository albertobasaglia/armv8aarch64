CC = aarch64-none-elf-gcc
LD = aarch64-none-elf-ld
AS = aarch64-none-elf-as

all: init.elf

init.elf: init.c linker.ld
	$(CC) -c init.c -o init.o
	$(LD) -Tlinker.ld -z max-page-size=0x1000 init.o -o init.elf

clean:
	rm -f init.o init.elf
