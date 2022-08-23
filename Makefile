all: kernel/kernel.elf hdd

kernel/kernel.elf: FORCE
	$(MAKE) -C kernel/

hdd: userspace/init.elf
	qemu-img create hdd 32M
	mkfs.fat -F16 hdd
	mkdir mount
	sudo mount -o loop hdd mount/
	sudo cp userspace/init.elf mount/INIT.ELF
	sudo umount mount/

userspace/init.elf: FORCE
	$(MAKE) -C userspace/ init.elf

clean:
	$(MAKE) -C kernel/ clean
	$(MAKE) -C userspace/ clean
	rm -f hdd
	rm -rf mount

FORCE: ;
