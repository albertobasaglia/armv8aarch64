#include "block.h"
#include "job.h"
#include "paging.h"
#include "sysutils.h"
#include "user.h"
#include "virtioblk.h"
#include <stddef.h>
#include <stdint.h>

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

struct slab paging_slab;

void handle_timer_int(int id, void* arg)
{
	klog("Ack TIMER");
	timer_write_tval(timer_getfrequency());
}

void enable_gic()
{
	gic_distributor_enable();
	gic_redistributor_wake();
	gic_interface_init();
	gic_interface_enablegroups();
	gic_interface_setprioritymask(0xff);
	gic_redistributor_enable_id(30);
	gic_distributor_set_priority(79, 0);
	gic_distributor_set_target(79, 1);
	gic_distributor_enable_id(79);
	gic_distributor_set_group(79, 0);
	gic_redistributor_set_handler(30, handle_timer_int, NULL);
}

void setup_heap()
{
	int heap_size_blocks = 1024; // 4MB
	size_t table_address = (size_t)&HEAP_START;
	size_t heap_address = table_address + heap_table_size(heap_size_blocks);
	sysutils_kernel_heap_create(table_address, heap_address,
				    heap_size_blocks);
}

void enable_paging_test()
{
	struct paging_manager pm;
	paging_manager_init(&pm, &paging_slab);
	paging_manager_map_kernel(&pm);
	paging_manager_apply(&pm);
	klog("Paging enabled");
}

void jump_usermode()
{
	struct paging_manager pm;
	paging_manager_init(&pm, &paging_slab);
	paging_manager_map_kernel(
	    &pm); // every userprocess has the kernel mapped in!
	paging_manager_map_1gb(&pm, 0x80000000, 0x80000000, 1, 0);
	struct job user_job = job_create((uint64_t)init, 0, "init", &pm);
	sysutils_jump_eret_usermode(&user_job);
}

void try_disk()
{
	struct virtioblk disk;
	if (disk_init(&disk, 31) == 0) {
		klog("Disk init is successful");
	} else {
		klog("Disk init error!");
	}

	struct block disk_block = disk_register_block_device(&disk);

	char buffer[512];

	block_read(&disk_block, buffer, 0);
	klog(buffer);
}

void start()
{
	sysutils_set_vbar((uint64_t)&EXCEPTION_TABLE);
	setup_heap();

	void* slab_memory = kalloc(
	    slab_get_needed_size(sizeof(struct page_table_store), 128));

	klogf("Slab memory allocated at 0x%x", slab_memory);

	paging_slab = slab_create(sizeof(struct page_table_store), 128,
				  slab_memory);

	sysutils_mask_fiq(false);
	sysutils_mask_irq(false);

	klog("Kernel started");
	enable_gic();
	klog("GIC enabled");

	timer_write_tval(timer_getfrequency());
	timer_enable();

	enable_paging_test();

	klog("Kernel finished");

	try_disk();
	while (1)
		;

	jump_usermode();
}
