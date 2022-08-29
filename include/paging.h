/*
 * DISCLAMER: This implementation contains a ton of magic numbers and hardcoded
 * stuff which is extremely bound to the architecture. Since this is a very
 * complex part of the kernel, I want a simple implementation to start with.
 * */
#ifndef PAGING_H
#define PAGING_H

#include <stdbool.h>
#include <stdint.h>

#include <slab.h>

#define PAGING_MAX_TABLES              0x1000

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
uint64_t paging_l012_create_entry_block(uint64_t block_address,
					int level,
					bool unprivileged_access,
					bool read_only);

/*
 * Returns an entry pointing to a table for a level 0, 1 or 2.
 * */
uint64_t paging_l012_create_entry_table(uint64_t table_address);

uint64_t paging_l3_create_entry_block(uint64_t block_address,
				      bool unprivileged_access,
				      bool read_only);

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

/*
 * Entries can both point to the next page table or to a block of memory.
 * */
#define PAGING_ENTRIES_PER_TABLE 512
union page_table_store {
	struct {
		uint64_t block[PAGING_ENTRIES_PER_TABLE];
	};
	struct {
		union page_table_store* next_table[PAGING_ENTRIES_PER_TABLE];
	};
};

struct paging_manager {
	union page_table_store* l1;
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

/*
 * Maps 1gb (1 entry in level 1 paging)
 * */
void paging_manager_map_1gb(struct paging_manager* paging_manager,
			    uint64_t va,
			    uint64_t pa,
			    bool unprivileged_access,
			    bool read_only);

/*
 * Manually maps a page
 * */
int paging_manager_map_page(struct paging_manager* paging_manager,
			    uint64_t va,
			    uint64_t pa,
			    bool unprivileged_access,
			    bool read_only);

/*
 * Splits address in offsets inside the page tables.
 * If passed pointer is NULL, that part is not calculated.
 * */
void paging_split_address(uint64_t address,
			  uint64_t* l1,
			  uint64_t* l2,
			  uint64_t* l3,
			  uint64_t* off);

int paging_try_map(union page_table_store* page_table_store,
		   uint64_t offset,
		   uint64_t value);

/*
 * Allocates a new table and sets all the entries to invalid.
 * */
union page_table_store* paging_allocate_table(struct paging_manager* pm);

int paging_insert_or_alloc(struct paging_manager* paging_manager,
			   union page_table_store* page_table_store,
			   int level,
			   uint64_t va,
			   uint64_t pa,
			   bool unprivileged_access,
			   bool read_only);

void paging_init_slab();

struct slab* paging_get_slab();


#endif
