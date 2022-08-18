#include <stddef.h>
#include <stdint.h>

#include <gic.h>
#include <heap.h>
#include <log.h>
#include <timer.h>
#include <uart.h>

extern char USERSTACK_END;
extern char EXCEPTION_TABLE;
extern char HEAP_START;

struct heap main_heap;

int get_current_el()
{
	uint64_t CurrentEL;
	asm inline("mrs %0, CurrentEL" : "=r"(CurrentEL));
	return (CurrentEL >> 2) & 0b11;
}

void user()
{
	put_string("I'm in usermode\n");
	while (1)
		;
}

void set_el0_sp()
{
	asm volatile("mov x1, %0\n"
		     "msr SP_EL0, x1" ::"r"(&USERSTACK_END)
		     : "x1");
}

void get_vbar_el1()
{
	uint64_t vbar_el1;
	asm volatile("mrs %0, VBAR_EL1" : "=r"(vbar_el1));
}

void handle_timer_int(int id)
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
	gic_redistributor_set_handler(30, handle_timer_int);
}

void set_vbar_el1()
{
	asm volatile("msr VBAR_EL1, %0" ::"r"(&EXCEPTION_TABLE));
}

void setup_heap()
{
	int heap_size_blocks = 256; // 1MB
	size_t table_address = (size_t)&HEAP_START;
	size_t heap_address = table_address + heap_table_size(heap_size_blocks);
	main_heap = heap_createtable((char*)heap_address, (char*)table_address,
				     heap_size_blocks);
	klogf("Created table at %x", table_address);
	klogf("Created heap at %x", heap_address);
}

void enable_paging_test()
{
	// 0b010 40 bits, 1TB.
	// 0b101 48 bits, 256TB

	uint64_t tcr_ips = 0b10; // 1TB output address!
	tcr_ips = tcr_ips << 32;

	tcr_ips |= 32; // t0sz

	asm volatile("mrs x0, TCR_EL1\n"
		     "orr x0, x0, %0\n"
		     "msr TCR_EL1, x0" ::"r"(tcr_ips)
		     : "x0");

	volatile uint64_t* l1 = (uint64_t*)0x42000000;
	for (int i = 0; i < 512; i++) {
		l1[i] = 0;
	}
	l1[0] = 0x00000001;
	l1[0] |= (1 << 10);
	l1[1] = 0x40000001;
	l1[1] |= (1 << 10);
	/* l1[2] = 0x40000001; */
	/* l1[2] |= (1 << 10); */

	asm volatile("msr TTBR0_EL1, %0" : "=r"(l1));

	asm volatile("TLBI VMALLE1"); // invalidate tlb

	asm volatile("mrs x0, SCTLR_EL1\n"
		     "orr x0, x0, #1\n"
		     "msr SCTLR_EL1, x0"); // everything breaks!

	klog("Paging enabled!");

	char* a = (char*)0x80000000;
	char* b = (char*)0x40000000;
	*a = 'x';
	put_char(*b);
}

void start()
{
	set_vbar_el1();
	setup_heap();

	// disable FIQ masking
	uint64_t flags = (1 << 6) | (1 << 7);
	flags = ~flags;
	asm volatile("mrs x0, DAIF\n"
		     "and x0, x0, %0\n"
		     "msr DAIF, x0" ::"r"(flags)
		     : "x0");
	klog("Kernel started");
	enable_gic();

	timer_write_tval(timer_getfrequency());
	timer_enable();

	enable_paging_test();

	klog("Kernel finished");
	uint64_t cnt_end = timer_read_systemcounter();
}
