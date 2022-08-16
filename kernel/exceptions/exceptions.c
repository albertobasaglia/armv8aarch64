#include "gic.h"
#include "timer.h"
#include "uart.h"
#include <exceptions.h>
#include <stdint.h>

void exceptions_distributor()
{
	put_string("Distributor!\n");
	uint64_t esr = exceptions_getesr();
}

void exceptions_handle_fiq()
{
	put_string("FIQ!\n");
	int intid = gic_interface_read_and_ack_group0();
	if (intid == 30)
		timer_disable();
	gic_interface_end_of_interrupt_group0(intid);
}

uint64_t exceptions_getesr()
{
	uint64_t esr;
	asm volatile("mrs %0, ESR_EL1" : "=r"(esr));
	return esr;
}
