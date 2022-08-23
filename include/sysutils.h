#ifndef SYSUTILS_H
#define SYSUTILS_H

#include <job.h>
#include <paging.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DAIF_IRQ_MASK 7
#define DAIF_FRQ_MASK 6

void sysutils_mask_irq(bool mask);

void sysutils_mask_fiq(bool mask);

void sysutils_set_vbar(uint64_t base_address);

void sysutils_jump_eret_usermode(struct job* job);

/*
 * Allocats kernel heap and keeps a pointer to it.
 * */
void sysutils_kernel_heap_create(size_t table_address,
				 size_t heap_address,
				 size_t heap_blocks_size);

void sysutils_kernel_heap_destroy();

void* sysutils_kernel_heap_alloc(size_t size);

void sysutils_kernel_heap_free(void* ptr);

void* kalloc(size_t size);

void kfree(void* ptr);

#endif
