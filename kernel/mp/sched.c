#include <gic.h>
#include <log.h>
#include <mp/job.h>
#include <mp/sched.h>
#include <paging.h>
#include <stdint.h>
#include <timer.h>

extern uint64_t* exceptions_context_switch_x30;

void debug_job(struct job* job)
{
	klogf("pc: 0x%x", job->pc);
	klogf("sp: 0x%x", job->sp);
	for (int i = 0; i <= 30; i++) {
		klogf("x%q: %x", i, job->x[i]);
	}
}

void scheduling_save_context(uint64_t* x30, struct job* job)
{
	uint64_t program_counter;
	uint64_t stack_pointer;
	asm volatile("mrs %0, ELR_EL1\n"
		     "mrs %1, SP_EL0"
		     : "=r"(program_counter), "=r"(stack_pointer));

	job_set_programcounter(job, program_counter);
	job->sp = stack_pointer;
	for (int i = 0; i <= 30; i++) {
		job->x[i] = x30[30 - i];
	}
}

void scheduling_restore_context(uint64_t* x30, struct job* job)
{
	uint64_t program_counter = job->pc;
	uint64_t stack_pointer = job->sp;

	for (int i = 0; i <= 30; i++) {
		x30[30 - i] = job->x[i];
	}

	asm volatile("msr ELR_EL1, %0\n"
		     "msr SP_EL0, %1" ::"r"(program_counter),
		     "r"(stack_pointer));
}

void internal_handler(int intid, void* args)
{
	timer_write_tval(timer_getfrequency()); // RELOAD THE TIMER

	scheduling_save_context(exceptions_context_switch_x30,
				job_get_current());

	job_forward();

	scheduling_restore_context(exceptions_context_switch_x30,
				   job_get_current());

	paging_manager_apply(job_get_current()->paging);
}

void scheduling_register_routine()
{
	gic_redistributor_set_handler(30, internal_handler, NULL);
}
