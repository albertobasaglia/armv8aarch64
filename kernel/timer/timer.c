#include <stdint.h>
#include <timer.h>

uint32_t timer_getfrequency()
{
	uint64_t frequency;
	asm volatile("mrs %0, CNTFRQ_EL0" : "=r"(frequency));
	return frequency;
}

void timer_enable()
{
	uint64_t enable_bit = (1 << 0);
	asm volatile("mrs x0, CNTP_CTL_EL0\n"
		     "orr x0, x0, %0\n"
		     "msr CNTP_CTL_EL0, x0" ::"r"(enable_bit)
		     : "x0");
}

void timer_write_tval(uint32_t tval)
{
	uint64_t write_value = tval;
	asm volatile("msr CNTP_TVAL_EL0, %0" ::"r"(write_value));
}
