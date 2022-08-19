#ifndef PAGING_H
#define PAGING_H

#include "slab.h"
#include <stdint.h>

#define PAGING_L012_VALID              (1 << 0)
#define PAGING_L012_TABLE              (1 << 1)
// L0 doesn't accept a block
#define PAGING_L1_BLOCK_ADDRESS_MASK   (0x3ffffull << 30)
#define PAGING_L2_BLOCK_ADDRESS_MASK   (0x7ffffffull << 21)
#define PAGING_L012_TABLE_ADDRESS_MASK (0xfffffffffull << 12)

#define PAGING_L3_VALID                (0b11ull << 0)
#define PAGING_L3_ADDRESS_MASK         (0xfffffffffull << 12)

#define PAGING_AP2                     (1 << 7)
#define PAGING_AP1                     (1 << 6)
#define PAGING_AF                      (1 << 10)
#define PAGING_nG                      (1 << 11)

/*
 * Returns an entry pointing to a block for a level 0, 1 or 2.
 * */
uint64_t paging_l012_create_entry_block(uint64_t block_address, int level);

/*
 * Returns an entry pointing to a table for a level 0, 1 or 2.
 * */
uint64_t paging_l012_create_entry_table(uint64_t table_address);

/*
 * Creates an invalid entry
 * */
uint64_t paging_create_entry_invalid();

/*
 * Sets the TTBR0 register
 * */
void paging_set_ttbr0(uint64_t table_address);

/*
 * Sets the TCR register
 *
 * TODO parametrize
 * */
void paging_set_tcr();

/*
 * Enables paging
 * */
void paging_enable();

#define PAGING_ENTRIES_PER_TABLE 512
struct page_table_store {
	uint64_t entries[PAGING_ENTRIES_PER_TABLE];
};

struct paging_manager {
	struct page_table_store* l1;
	struct slab* pages_allocator;
};

/*
 * Configures registers to use this paging scheme.
 * */
void paging_manager_apply(struct paging_manager* paging_manager);

/*
 * Inits the struct and configures the first level page.
 * By default all entries are invalid.
 * */
void paging_manager_init(struct paging_manager* paging_manager,
			 struct slab* pages_allocator);

/*
 * Maps the kernel in the page table
 * */
void paging_manager_map_kernel(struct paging_manager* paging_manager);

#endif
