CFLAGS = -g -ffreestanding -fno-builtin -fno-builtin-function -mcpu=cortex-a53 \
	 -I../include/
# ASFLAGS =
LDFLAGS = -T linker.ld

CC = aarch64-none-elf-gcc
LD = aarch64-none-elf-ld
AS = aarch64-none-elf-as

BUILD_OBJS = timer/timer.o \
	     uart.o \
	     gic/gic.o \
	     exceptions/exceptions.o \
	     alloc/slab.o \
	     alloc/heap.o \
	     string.o \
	     log.o \
	     mm/paging.o \
	     sysutils.o \
	     job.o \
	     user.o

all: kernel.elf

kernel.elf: _start.o linker.ld start.o _exceptions.o $(BUILD_OBJS)
	$(LD) $(LDFLAGS) _start.o start.o _exceptions.o $(BUILD_OBJS) -o kernel.elf

_start.o: start.asm
	$(AS) start.asm -o _start.o

_exceptions.o: exceptions.asm
	$(AS) exceptions.asm -o _exceptions.o

start.o: start.c
	$(CC) $(CFLAGS) -c start.c -o start.o

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f _start.o start.o _exceptions.o $(BUILD_OBJS) kernel.elf
