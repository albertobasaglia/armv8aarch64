#include <block.h>
#include <elf/elf.h>
#include <elf/filebuffer.h>
#include <fs/fat.h>
#include <fs/fat_fs.h>
#include <fs/fs.h>
#include <fs/ram_fs.h>
#include <mp/job.h>
#include <mp/sched.h>
#include <paging.h>
#include <stddef.h>
#include <stdint.h>
#include <sysutils.h>
#include <virtioblk.h>

#include <gic.h>
#include <heap.h>
#include <log.h>
#include <slab.h>
#include <string.h>
#include <timer.h>
#include <uart.h>

extern char USERSTACK_END;
extern char EXCEPTION_TABLE;
extern char HEAP_START;

void enable_gic()
{
	gic_distributor_enable();
	gic_redistributor_wake();
	gic_interface_init();
	gic_interface_enablegroups();
	gic_interface_setprioritymask(0xff);
	gic_redistributor_enable_id(30);
	// 79 is the virtioblk interrupt
	gic_distributor_set_priority(79, 0);
	gic_distributor_set_target(79, 1);
	gic_distributor_enable_id(79);
	gic_distributor_set_group(79, 0);
}

void setup_heap()
{
	int heap_size_blocks = 0x8000; // 128MB
	size_t table_address = (size_t)&HEAP_START;
	size_t heap_address = table_address + heap_table_size(heap_size_blocks);
	sysutils_kernel_heap_create(table_address, heap_address,
				    heap_size_blocks);
}

void enable_paging_test()
{
	paging_manager_init(sysutils_get_paging_kernel(), paging_get_slab());
	paging_manager_map_kernel(sysutils_get_paging_kernel());
	paging_manager_apply(sysutils_get_paging_kernel());
	klog("Paging enabled");
}

struct block disk_block;

void usermode()
{
	// Read the elf file from the disk
	struct virtioblk disk;
	if (disk_init(&disk, 31) == 0) {
		klog("Disk init is successful");
	} else {
		klog("Disk init error!");
	}
	disk_block = disk_register_block_device(&disk);

	struct filesystem* fs = fatfs_createfs(&disk_block);
	fs_filesystem_setmain(fs);

	struct inode* inode = fs->open(fs, "init.elf");

	job_create_from_file(inode, "init");

	scheduling_register_routine();
	sysutils_log_free_heap();
	sysutils_jump_eret_usermode(job_get_current());
}

void start()
{
	klog("Kernel started");

	sysutils_set_vbar((uint64_t)&EXCEPTION_TABLE);

	setup_heap();
	fs_init_slab();
	paging_init_slab();
	job_init_slab(128);

	sysutils_mask_fiq(false);
	sysutils_mask_irq(false);

	enable_gic();
	klog("GIC enabled");

	timer_write_tval(timer_getfrequency());
	timer_enable();

	enable_paging_test();

	usermode();
	/* testfs(); */
	/* debug_paging(); */
	/* testfs(); */
	/* testramfs(); */
}
