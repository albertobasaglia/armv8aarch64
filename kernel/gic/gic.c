#include <gic.h>

#include <stdint.h>

void gic_distributor_enable()
{
	uint32_t ctrl = 0;
	volatile uint32_t* GICD_CTRL = (uint32_t*)GIC_DIST + 0x0;
	ctrl |= (1 << 4); // ARE (Affinity Register Enable)
	*GICD_CTRL = ctrl;

	// enable groups
	ctrl |= (1 << 2);
	ctrl |= (1 << 1);
	ctrl |= (1 << 0);
	*GICD_CTRL = ctrl;
}

void gic_redistributor_wake()
{
	volatile uint32_t* GICR_WAKER = (uint32_t*)(GIC_REDIST + 0x14);
	*GICR_WAKER &= ~(1 << 1);

	// polls for wake complete
	while (*GICR_WAKER & (1 << 2))
		;
}

void gic_redistributor_enable_id(int id)
{
	// TODO this works for 0-31 only atm
	volatile uint32_t* GICR_ISENABLER0 = (uint32_t*)(GIC_REDIST_SGI +
							 0x100);

	*GICR_ISENABLER0 |= (1 << id);
}

void gic_interface_setbinarypoint0(uint8_t bp)
{
	uint64_t bp64 = bp & 0b111;
	asm volatile("mov x0, %0\n"
		     "msr ICC_BPR0_EL1, x0" ::"r"(bp64)
		     : "x0");
}

void gic_interface_setbinarypoint1(uint8_t bp)
{
	uint64_t bp64 = bp & 0b111;
	asm volatile("mov x0, %0\n"
		     "msr ICC_BPR1_EL1, x0" ::"r"(bp64)
		     : "x0");
}

void gic_interface_setprioritymask(uint8_t pmr)
{
	uint64_t pmr64 = pmr;
	asm volatile("mov x0, %0\n"
		     "msr ICC_PMR_EL1, x0" ::"r"(pmr64)
		     : "x0");
}

void gic_interface_enablegroups()
{
	asm volatile("mov x0, #1\n"
		     "msr ICC_IGRPEN0_EL1, x0\n"
		     "msr ICC_IGRPEN1_EL1, x0");
}

void gic_interface_init()
{
	asm volatile("mov x0, #1\n"
		     "msr ICC_SRE_EL1, x0");
}
