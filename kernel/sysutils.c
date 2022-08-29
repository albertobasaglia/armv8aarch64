#include <heap.h>
#include <log.h>
#include <paging.h>
#include <stdint.h>
#include <string.h>
#include <sysutils.h>

static struct heap main_heap;
static struct paging_manager paging_kernel;

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
	uint64_t target_el_mask = 0xf;
	target_el_mask = ~target_el_mask;
	uint64_t target_el = 0; // EL0
				// set the return level (and stack pointer)
	asm volatile("mrs x0, SPSR_EL1\n"
		     "and x0, x0, %0\n"
		     "orr x0, x0, %1\n"
		     "msr SPSR_EL1, x0" ::"r"(target_el_mask),
		     "r"(target_el)
		     : "x0");
	paging_manager_apply(job->paging);
	asm volatile("msr ELR_EL1, %0\n"
		     "msr SP_EL0, %1\n"
		     "eret" ::"r"(job->pc),
		     "r"(job->sp));
}

void sysutils_kernel_heap_create(size_t table_address,
				 size_t heap_address,
				 size_t heap_blocks_size)
{
	main_heap = heap_createtable((char*)heap_address, (char*)table_address,
				     heap_blocks_size);
}

void sysutils_kernel_heap_destroy()
{
	// TODO
}

void* sysutils_kernel_heap_alloc(size_t size)
{
	return heap_alloc(&main_heap, size);
}

void sysutils_kernel_heap_free(void* ptr)
{
	heap_free(&main_heap, ptr);
}

void* kalloc(size_t size)
{
	return sysutils_kernel_heap_alloc(size);
}

void* kzalloc(size_t size)
{
	void* res = sysutils_kernel_heap_alloc(size);
	memset(res, 0, size);
	return res;
}

void kfree(void* ptr)
{
	sysutils_kernel_heap_free(ptr);
}

size_t sysutils_kernel_heap_get_free_count()
{
	return heap_get_free_blocks(&main_heap);
}

size_t sysutils_kernel_heap_get_total_count()
{
	return main_heap.blocks;
}

void sysutils_log_free_heap()
{
	klogf("Free blocks: %q/%q", sysutils_kernel_heap_get_free_count(),
	      sysutils_kernel_heap_get_total_count());
}

struct paging_manager* sysutils_get_paging_kernel()
{
	return &paging_kernel;
}
