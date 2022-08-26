#include <gic.h>
#include <log.h>
#include <slab.h>

#include <stdint.h>

#define MAX_INTS 1024

static Handler* handlers[MAX_INTS];
static void* handler_params[MAX_INTS];

void gic_distributor_enable()
{
	uint32_t ctrl = 0;
	volatile uint32_t* GICD_CTRL = (uint32_t*)(GIC_DIST + 0x0);
	ctrl |= (1 << 4); // ARE (Affinity Register Enable)
	*GICD_CTRL = ctrl;

	// enable groups
	ctrl |= (1 << 2);
	ctrl |= (1 << 1);
	ctrl |= (1 << 0);
	*GICD_CTRL = ctrl;
}

void gic_distributor_enable_id(int intid)
{
	int offset = intid % 32;
	int reg = intid / 32;

	volatile uint32_t* GICD_ISENABLER = (uint32_t*)(GIC_DIST + 0x100);
	GICD_ISENABLER += reg;

	*GICD_ISENABLER |= (1 << offset);
}

void gic_distributor_set_group(int id, int group)
{
	int offset = id % 32;
	int reg = id / 32;

	volatile uint32_t* GICD_ISENABLER = (uint32_t*)(GIC_DIST + 0x80);
	GICD_ISENABLER += reg;

	if (group) {
		*GICD_ISENABLER |= (1 << offset);
	} else {
		*GICD_ISENABLER &= ~(1 << offset);
	}
}

void gic_distributor_set_priority(int id, uint8_t priority)
{
	int offset = id % 4;
	int reg = id / 4;

	uint32_t shifted = priority << (8 * offset);
	uint32_t mask = 0xff << (8 * offset);
	mask = ~mask;

	volatile uint32_t* GICD_ISENABLER = (uint32_t*)(GIC_DIST + 0x400);
	GICD_ISENABLER += reg;

	*GICD_ISENABLER &= mask;
	*GICD_ISENABLER |= shifted;
}

void gic_distributor_set_target(int id, uint8_t value)
{
	int offset = id % 4;
	int reg = id / 4;

	uint32_t shifted = value << (8 * offset);
	uint32_t mask = 0xff << (8 * offset);
	mask = ~mask;

	volatile uint32_t* GICD_ISENABLER = (uint32_t*)(GIC_DIST + 0x800);
	GICD_ISENABLER += reg;

	*GICD_ISENABLER &= mask;
	*GICD_ISENABLER |= shifted;
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
	int offset = id % 32;
	int reg = id / 32;

	volatile uint32_t* GICR_ISENABLER0 = (uint32_t*)(GIC_REDIST_SGI +
							 0x100);
	GICR_ISENABLER0 += reg;

	*GICR_ISENABLER0 |= (1 << offset);
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

uint32_t gic_interface_read_and_ack_group0()
{
	uint64_t iar;
	asm volatile("mrs %0, ICC_IAR0_EL1" : "=r"(iar));
	return iar & 0xffffff;
}

uint32_t gic_interface_read_and_ack_group1()
{
	uint64_t iar;
	asm volatile("mrs %0, ICC_IAR1_EL1" : "=r"(iar));
	return iar & 0xffffff;
}

void gic_interface_end_of_interrupt_group0(uint32_t intid)
{
	uint64_t reg = intid & 0xffffff;
	asm volatile("msr ICC_EOIR0_EL1, %0" ::"r"(reg));
}

void gic_interface_end_of_interrupt_group1(uint32_t intid)
{
	uint64_t reg = intid & 0xffffff;
	asm volatile("msr ICC_EOIR1_EL1, %0" ::"r"(reg));
}

void gic_redistributor_set_handler(int intid, Handler* handler, void* arg)
{
	if (intid < 0 || intid > MAX_INTS)
		return;

	handlers[intid] = handler;
	handler_params[intid] = arg;
}

Handler* gic_redistributor_get_handler(int intid)
{
	if (intid < 0 || intid > MAX_INTS)
		return NULL;

	return handlers[intid];
}

void* gic_redistributor_get_handler_param(int intid)
{
	if (intid < 0 || intid > MAX_INTS)
		return NULL;

	return handler_params[intid];
}

void gic_redistributor_erase_handlers()
{
	for (int i = 0; i < MAX_INTS; i++)
		handlers[i] = NULL;
}
