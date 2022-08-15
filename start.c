#include <stddef.h>
#include <stdint.h>

#define UART_BASEADDRESS 0x09000000

#define GIC_DIST         0x08000000

#define GIC_CPU          0x08010000

#define GIC_REDIST       0x080a0000
#define GIC_REDIST_SGI   0x080b0000

extern char USERSTACK_END;

int get_current_el()
{
	uint64_t CurrentEL;
	asm inline("mrs %0, CurrentEL" : "=r"(CurrentEL));
	return (CurrentEL >> 2) & 0b11;
}

void putchar(char c)
{
	static char* dest = (char*)UART_BASEADDRESS;
	*dest = c;
}

void putstring(char* str)
{
	while (*str != 0) {
		putchar(*str);
		str++;
	}
}

void timer_test()
{
	size_t freq;
	asm volatile("mrs %0, CNTFRQ_EL0\n" : "=r"(freq));

	asm volatile("msr CNTP_TVAL_EL0, %0" ::"r"(freq));

	// enable timer
	asm volatile("mrs x0, CNTP_CTL_EL0\n"
		     "orr x0, x0, #0b1\n"
		     "msr CNTP_CTL_EL0, x0");
}

void user()
{
	putstring("I'm in usermode!\n");
	asm volatile("svc 17");
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
	ctrl |= (1 << 4); // ARE
	*GICD_CTRL = ctrl;

	ctrl |= (1 << 2);
	ctrl |= (1 << 1);
	ctrl |= (1 << 0);
	*GICD_CTRL = ctrl;

	volatile uint32_t* GICR_WAKER = (uint32_t*)(GIC_REDIST + 0x14);
	*GICR_WAKER &= ~(1 << 1);

	while (*GICR_WAKER & (1 << 2))
		;

	putstring("Activating GICC\n");

	volatile uint32_t* GICD_ISENABLER0 = (uint32_t*)(GIC_DIST + 0x100);
	volatile uint32_t* GICD_ISPENDR0 = (uint32_t*)(GIC_DIST + 0x200);
	volatile uint32_t* GICD_ISACTIVER0 = (uint32_t*)(GIC_DIST + 0x300);

	volatile uint32_t* GICR_ISENABLER0 = (uint32_t*)(GIC_REDIST_SGI +
							 0x100);

	*GICR_ISENABLER0 |= (1 << 30); // THIS MADE IT WORK!!!

	// TODO this causes a data abort!
	/* volatile uint32_t* GICC_CTRL = (uint32_t*)(GIC_CPU + 0x0); */
	/* *GICC_CTRL |= (0b11); */

	asm volatile("mov x0, #1\n"
		     "msr ICC_SRE_EL1, x0");

	asm volatile("mov x0, #0\n"
		     "msr ICC_BPR0_EL1, x0\n"
		     "msr ICC_BPR1_EL1, x0");

	asm volatile("mov x0, #1\n"
		     "msr ICC_IGRPEN0_EL1, x0\n"
		     "msr ICC_IGRPEN1_EL1, x0");

	asm volatile("mov x0, #0xff\n"
		     "msr ICC_PMR_EL1, x0");

	putstring("Activated GICC\n");

	*GICD_ISENABLER0 |= (1 << 30);
}

void start()
{
	uint64_t flags = (1 << 6) | (1 << 7);
	flags = ~flags;
	asm volatile("mrs x0, DAIF\n"
		     "and x0, x0, %0\n"
		     "msr DAIF, x0" ::"r"(flags)
		     : "x0");
	putstring("Kernel started!\n");
	enable_gic();
	putstring("Kernel finished!\n");
	/* set_el0_sp(); */
	/* get_vbar_el1(); */

	// jump to usermode:
	/* asm volatile("mov x1, %0\n" */
	/* 	     "msr ELR_EL1, x1\n" */
	/* 	     "eret" ::"r"(user) */
	/* 	     : "x1"); */
	timer_test();
}
