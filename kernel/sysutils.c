#include "log.h"
#include "paging.h"
#include <stdint.h>
#include <sysutils.h>

void sysutils_daif_set(int bit)
{
	uint64_t bit_set = (1 << bit);
	asm volatile("mrs x0, DAIF\n"
		     "orr x0, x0, %0\n"
		     "msr DAIF, x0" ::"r"(bit_set)
		     : "x0");
}

void sysutils_daif_clear(int bit)
{
	uint64_t bit_clear = ~(1 << bit);
	asm volatile("mrs x0, DAIF\n"
		     "and x0, x0, %0\n"
		     "msr DAIF, x0" ::"r"(bit_clear)
		     : "x0");
}

void sysutils_mask_irq(bool mask)
{
	if (mask)
		sysutils_daif_set(DAIF_IRQ_MASK);
	else
		sysutils_daif_clear(DAIF_IRQ_MASK);
}

void sysutils_mask_fiq(bool mask)
{
	if (mask)
		sysutils_daif_set(DAIF_FRQ_MASK);
	else
		sysutils_daif_clear(DAIF_FRQ_MASK);
}

void sysutils_set_vbar(uint64_t base_address)
{
	asm volatile("msr VBAR_EL1, %0" ::"r"(base_address));
}

void sysutils_jump_eret_usermode(struct job* job)
{
	paging_manager_apply(job->paging);
	asm volatile("msr ELR_EL1, %0\n"
		     "msr SP_EL0, %1\n"
		     "eret" ::"r"(job->pc),
		     "r"(job->sp));
}
