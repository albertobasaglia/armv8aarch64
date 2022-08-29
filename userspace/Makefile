CC = aarch64-none-elf-gcc -ggdb
LD = aarch64-none-elf-ld
AS = aarch64-none-elf-as

all: init.elf hello.elf

init.elf: init.c linker.ld runtime.o
	$(CC) -c init.c -o init.o
	$(LD) -Tlinker.ld -z max-page-size=0x1000 runtime.o init.o -o init.elf

hello.elf: hello.c linker.ld runtime.o
	$(CC) -c hello.c -o hello.o
	$(LD) -Tlinker.ld -z max-page-size=0x1000 runtime.o hello.o -o hello.elf

runtime.o: runtime.c
	$(CC) -c runtime.c -o runtime.o

clean:
	rm -f init.o init.elf hello.o hello.elf
