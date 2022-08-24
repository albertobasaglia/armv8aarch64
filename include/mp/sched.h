#ifndef MP_SCHED_H
#define MP_SCHED_H

#include <mp/job.h>
#include <stdint.h>

/*
 * Registers the interrupt routine for the scheduler
 * */
void scheduling_register_routine();

/*
 * Saves the context on the job struct.
 *
 * - 31 registers (from the memory zone)
 * - program counter (from ELR_EL1)
 * - stack pointer (from SP_EL0)
 * */
void scheduling_save_context(uint64_t* x30, struct job* job);

/*
 * Restores the context from the job struct.
 *
 * - 31 registers (from the memory zone)
 * - program counter (from ELR_EL1)
 * - stack pointer (from SP_EL0)
 * */
void scheduling_restore_context(uint64_t* x30, struct job* job);

#endif
