#include "log.h"
#include <gic.h>
#include <stdbool.h>
#include <timer.h>
#include <uart.h>

#include <exceptions.h>
#include <stddef.h>
#include <stdint.h>

bool exceptions_handle_syscall(uint16_t imm)
{

	return 0;
}

void exceptions_distributor()
{
	uint64_t esr = exceptions_getesr();
	uint64_t ec = (esr >> ESR_EC_OFFSET) & ESR_EC_MASK;
	if (ec == ESR_EC_DATABT_SAME) {
		klog("Data abort from same EL");
	} else if (ec == ESR_EC_DATABT_LOWER) {
		klog("Data abort from lower EL");
	} else if (ec == ESR_EC_SVC64) {
		uint16_t imm16 = esr & ESR_ISS_SVC_IMM16;
		klogf("Requested system call imm16: %q", imm16);
		bool res = exceptions_handle_syscall(imm16);
		if (!res) {
			klogf("Unhandled syscall!");
			while (1)
				;
		}
	} else {
		klog("Unhandled exception!");
		while (1)
			;
	}
}

void exceptions_handle_fiq()
{
	int intid = gic_interface_read_and_ack_group0();
	Handler* handler = gic_redistributor_get_handler(intid);
	if (handler != NULL) {
		handler(intid);
	} else {
		put_string("Unhandled interrupt\n");
	}
	gic_interface_end_of_interrupt_group0(intid);
}

uint64_t exceptions_getesr()
{
	uint64_t esr;
	asm volatile("mrs %0, ESR_EL1" : "=r"(esr));
	return esr;
}
