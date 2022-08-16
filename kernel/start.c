#include "timer.h"
#include "uart.h"
#include <stddef.h>
#include <stdint.h>

#define GIC_DIST       0x08000000

#define GIC_CPU        0x08010000

#define GIC_REDIST     0x080a0000
#define GIC_REDIST_SGI 0x080b0000

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
	uint32_t ctrl = 0;
	volatile uint32_t* GICD_CTRL = (uint32_t*)GIC_DIST + 0x0;
	ctrl |= (1 << 4); // ARE (Affinity Register Enable)
	*GICD_CTRL = ctrl;

	ctrl |= (1 << 2);
	ctrl |= (1 << 1);
	ctrl |= (1 << 0);
	*GICD_CTRL = ctrl;

	volatile uint32_t* GICR_WAKER = (uint32_t*)(GIC_REDIST + 0x14);
	*GICR_WAKER &= ~(1 << 1);

	while (*GICR_WAKER & (1 << 2))
		;

	put_string("Activating GICC\n");

	volatile uint32_t* GICD_ISENABLER0 = (uint32_t*)(GIC_DIST + 0x100);
	volatile uint32_t* GICD_ISPENDR0 = (uint32_t*)(GIC_DIST + 0x200);
	volatile uint32_t* GICD_ISACTIVER0 = (uint32_t*)(GIC_DIST + 0x300);

	volatile uint32_t* GICR_ISENABLER0 = (uint32_t*)(GIC_REDIST_SGI +
							 0x100);

	// enable interrupt ID30
	*GICR_ISENABLER0 |= (1 << 30);

	// enable
	asm volatile("mov x0, #1\n"
		     "msr ICC_SRE_EL1, x0");

	// set binary point register
	asm volatile("mov x0, #0\n"
		     "msr ICC_BPR0_EL1, x0\n"
		     "msr ICC_BPR1_EL1, x0");
	// set priority max register to 0xff (lowest possible)
	asm volatile("mov x0, #0xff\n"
		     "msr ICC_PMR_EL1, x0");

	// enable groups 1 and 0
	asm volatile("mov x0, #1\n"
		     "msr ICC_IGRPEN0_EL1, x0\n"
		     "msr ICC_IGRPEN1_EL1, x0");

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
	/* set_el0_sp(); */
	/* get_vbar_el1(); */

	timer_write_tval(timer_getfrequency());
	timer_enable();

	// jump to usermode:
	/* asm volatile("mov x1, %0\n" */
	/* 	     "msr ELR_EL1, x1\n" */
	/* 	     "eret" ::"r"(user) */
	/* 	     : "x1"); */
}
