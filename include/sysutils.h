#ifndef SYSUTILS_H
#define SYSUTILS_H

#include <job.h>
#include <paging.h>

#include <stdbool.h>
#include <stdint.h>

#define DAIF_IRQ_MASK 7
#define DAIF_FRQ_MASK 6

void sysutils_mask_irq(bool mask);

void sysutils_mask_fiq(bool mask);

void sysutils_set_vbar(uint64_t base_address);

void sysutils_jump_eret_usermode(struct job* job);

#endif
