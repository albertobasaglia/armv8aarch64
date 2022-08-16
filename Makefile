all: kernel/kernel.elf

kernel/kernel.elf:
	$(MAKE) -C kernel/

clean:
	$(MAKE) -C kernel/ clean
