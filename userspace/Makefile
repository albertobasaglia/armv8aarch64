CC = aarch64-none-elf-gcc
LD = aarch64-none-elf-ld
AS = aarch64-none-elf-as

all: init.elf

init.elf: init.c linker.ld runtime.o
	$(CC) -c init.c -o init.o
	$(LD) -Tlinker.ld -z max-page-size=0x1000 runtime.o init.o -o init.elf

runtime.o: runtime.c
	$(CC) -c runtime.c -o runtime.o

clean:
	rm -f init.o init.elf
