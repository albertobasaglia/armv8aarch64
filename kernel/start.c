#include "paging.h"
#include <stddef.h>
#include <stdint.h>

#include <gic.h>
#include <heap.h>
#include <log.h>
#include <slab.h>
#include <timer.h>
#include <uart.h>

extern char USERSTACK_END;
extern char EXCEPTION_TABLE;
extern char HEAP_START;

struct heap main_heap;
struct slab paging_slab;

void handle_timer_int(int id)
{
	klog("Ack TIMER");
	timer_write_tval(timer_getfrequency());
}

void set_vbar_el1()
{
	asm volatile("msr VBAR_EL1, %0" ::"r"(&EXCEPTION_TABLE));
}

void enable_gic()
{
	gic_distributor_enable();
	gic_redistributor_wake();
	gic_interface_init();
	gic_interface_enablegroups();
	gic_interface_setprioritymask(0xff);
	gic_redistributor_enable_id(30);
	gic_redistributor_set_handler(30, handle_timer_int);
}

void setup_heap()
{
	int heap_size_blocks = 1024; // 4MB
	size_t table_address = (size_t)&HEAP_START;
	size_t heap_address = table_address + heap_table_size(heap_size_blocks);
	main_heap = heap_createtable((char*)heap_address, (char*)table_address,
				     heap_size_blocks);
	klogf("Created table at 0x%x", table_address);
	klogf("Created heap at 0x%x", heap_address);
}

void enable_paging_test()
{
	struct paging_manager pm;
	paging_manager_init(&pm, &paging_slab);
	paging_manager_map_kernel(&pm);
	paging_manager_apply(&pm);
	klog("Paging enabled");
}

void start()
{
	set_vbar_el1();
	setup_heap();

	void* slab_memory = heap_alloc(
	    &main_heap,
	    slab_get_needed_size(sizeof(struct page_table_store), 128));

	klogf("Slab memory allocated at 0x%x", slab_memory);

	paging_slab = slab_create(sizeof(struct page_table_store), 128,
				  slab_memory);

	// disable FIQ masking
	uint64_t flags = (1 << 6) | (1 << 7);
	flags = ~flags;
	asm volatile("mrs x0, DAIF\n"
		     "and x0, x0, %0\n"
		     "msr DAIF, x0" ::"r"(flags)
		     : "x0");
	klog("Kernel started");
	enable_gic();
	klog("GIC enabled");

	timer_write_tval(timer_getfrequency());
	timer_enable();

	enable_paging_test();

	klog("Kernel finished");
	uint64_t cnt_end = timer_read_systemcounter();
}
