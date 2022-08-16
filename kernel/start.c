#include "gic.h"
#include "timer.h"
#include "uart.h"
#include <stddef.h>
#include <stdint.h>

extern char USERSTACK_END;
extern char EXCEPTION_TABLE;

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

void enable_gic()
{
	put_string("Activating GICC\n");
	gic_distributor_enable();
	gic_redistributor_wake();
	gic_interface_init();
	gic_interface_enablegroups();
	gic_interface_setprioritymask(0xff);
	gic_redistributor_enable_id(30);
	put_string("Activated GICC\n");
}

void set_vbar_el1()
{
	asm volatile("msr VBAR_EL1, %0" ::"r"(&EXCEPTION_TABLE));
}

void start()
{
	set_vbar_el1();
	uint64_t flags = (1 << 6) | (1 << 7);
	flags = ~flags;
	asm volatile("mrs x0, DAIF\n"
		     "and x0, x0, %0\n"
		     "msr DAIF, x0" ::"r"(flags)
		     : "x0");
	put_string("Kernel started!\n");
	enable_gic();
	put_string("Kernel finished!\n");

	timer_write_tval(timer_getfrequency());
	timer_enable();

	// jump to usermode:
	/* asm volatile("mov x1, %0\n" */
	/* 	     "msr ELR_EL1, x1\n" */
	/* 	     "eret" ::"r"(user) */
	/* 	     : "x1"); */
}
