#include <log.h>
#include <paging.h>
#include <slab.h>
#include <stdbool.h>
#include <stdint.h>
#include <sysutils.h>

struct slab slab;
struct paging_manager* current_paging_manager = NULL;

uint64_t paging_l012_create_entry_block(uint64_t block_address,
					int level,
					bool unprivileged_access,
					bool read_only)
{
	if (level <= 0 || level > 2)
		return 0;

	uint64_t entry = PAGING_L012_VALID;
	/* entry |= block_address; is this necessary?*/

	if (unprivileged_access)
		entry |= PAGING_AP1;

	if (read_only)
		entry |= PAGING_AP2;

	if (level == 1)
		entry |= block_address & PAGING_L1_BLOCK_ADDRESS_MASK;
	else if (level == 2)
		entry |= block_address & PAGING_L2_BLOCK_ADDRESS_MASK;

	entry |= PAGING_AF;

	return entry;
}

uint64_t paging_l012_create_entry_table(uint64_t table_address)
{
	uint64_t entry = PAGING_L012_VALID | PAGING_L012_TABLE;
	entry |= table_address & PAGING_L012_TABLE_ADDRESS_MASK;
	return entry;
}

uint64_t paging_l3_create_entry_block(uint64_t block_address,
				      bool unprivileged_access,
				      bool read_only)
{
	uint64_t entry = PAGING_L3_VALID;
	entry |= block_address & PAGING_L3_ADDRESS_MASK;

	if (unprivileged_access)
		entry |= PAGING_AP1;
	if (read_only)
		entry |= PAGING_AP2;

	entry |= PAGING_AF;

	return entry;
}

uint64_t paging_create_entry_invalid()
{
	return 0;
}

void paging_set_ttbr0(uint64_t table_address)
{
	asm volatile("msr TTBR0_EL1, %0" ::"r"(table_address));
}

uint64_t paging_get_ttbr0()
{
	uint64_t table_address;
	asm volatile("mrs %0, TTBR0_EL1" : "=r"(table_address));
	return table_address;
}

void paging_set_tcr()
{
	uint64_t tcr_ips = 0b10;
	tcr_ips = tcr_ips << 32;
	tcr_ips |= 25; // using l1 with virtual space: 2^(64-25) = 512GB
	asm volatile("mrs x0, TCR_EL1\n"
		     "orr x0, x0, %0\n"
		     "msr TCR_EL1, x0" ::"r"(tcr_ips)
		     : "x0");
}

void paging_enable()
{
	asm volatile("mrs x0, SCTLR_EL1\n"
		     "orr x0, x0, #1\n"
		     "msr SCTLR_EL1, x0");
}

struct paging_manager* paging_manager_apply(
    struct paging_manager* paging_manager)
{
	struct paging_manager* old = current_paging_manager;
	current_paging_manager = paging_manager;
	paging_set_ttbr0((uint64_t)paging_manager->l1);
	paging_set_tcr();
	paging_enable();
	asm volatile("tlbi vmalle1");
	return old;
}

void paging_manager_init(struct paging_manager* paging_manager,
			 struct slab* pages_allocator)
{
	paging_manager->pages_allocator = pages_allocator;

	union page_table_store* l1 = slab_allocate(
	    paging_manager->pages_allocator);

	for (int i = 0; i < PAGING_ENTRIES_PER_TABLE; i++) {
		l1->block[i] = paging_create_entry_invalid();
	}

	paging_manager->l1 = l1;
}

void paging_manager_map_kernel(struct paging_manager* paging_manager)
{
	paging_manager_map_1gb(paging_manager, 0x0, 0x0, 0, 0);
	paging_manager_map_1gb(paging_manager, 0x40000000, 0x40000000, 0, 0);
}

void paging_manager_map_1gb(struct paging_manager* paging_manager,
			    uint64_t va,
			    uint64_t pa,
			    bool unprivileged_access,
			    bool read_only)
{
	int gb_offset = va / 0x40000000;

	paging_manager->l1->block[gb_offset] = paging_l012_create_entry_block(
	    pa, 1, unprivileged_access, read_only);
}

void paging_split_address(uint64_t address,
			  uint64_t* l1,
			  uint64_t* l2,
			  uint64_t* l3,
			  uint64_t* off)
{
	if (l1)
		*l1 = address / 0x40000000;
	address %= 0x40000000;

	if (l2)
		*l2 = address / 0x200000;
	address %= 0x200000;

	if (l3)
		*l3 = address / 0x1000;
	address %= 0x1000;

	if (off)
		*off = address;
}

int paging_manager_map_page(struct paging_manager* paging_manager,
			    uint64_t va,
			    uint64_t pa,
			    bool unprivileged_access,
			    bool read_only)
{
	int res = paging_insert_or_alloc(paging_manager, paging_manager->l1, 1,
					 va, pa, unprivileged_access,
					 read_only);

	if (res)
		return res;

	asm volatile("tlbi vmalle1\n"
		     "dsb sy");
	return 0;
}

int paging_try_map(union page_table_store* page_table_store,
		   uint64_t offset,
		   uint64_t value)
{
	uint64_t old_value = page_table_store->block[offset];

	if (old_value == value)
		return 0;

	if (old_value & 1)
		return 1;

	page_table_store->block[offset] = value;
	return 0;
}

union page_table_store* paging_allocate_table(struct paging_manager* pm)
{
	union page_table_store* allocated = slab_allocate(pm->pages_allocator);

	for (int i = 0; i < PAGING_ENTRIES_PER_TABLE; i++) {
		allocated->block[i] = 0;
	}

	return allocated;
}

int paging_insert_or_alloc(struct paging_manager* paging_manager,
			   union page_table_store* page_table_store,
			   int level,
			   uint64_t va,
			   uint64_t pa,
			   bool unprivileged_access,
			   bool read_only)
{
	if (level < 1) {
		/*
		 * Be aware that this is possible with some OAs configurations.
		 * */
		return -1;
	}

	uint64_t l1_offset;
	uint64_t l2_offset;
	uint64_t l3_offset;

	paging_split_address(va, &l1_offset, &l2_offset, &l3_offset, NULL);

	if (level < 3) {
		/*
		 * Level 1
		 * Level 2
		 * This points to the next table
		 * */
		uint64_t offset;
		if (level == 1)
			offset = l1_offset;
		else
			offset = l2_offset;

		uint64_t old_value = page_table_store->block[offset];
		union page_table_store* pts;

		if ((old_value & 0b11) == 0b01)
			return -1; // The slot is already occupied by a block!

		if ((old_value & 0b1) == 0) {
			// A new page has to be allocated
			pts = paging_allocate_table(paging_manager);

			// Save the new page entry in the table

			uint64_t new_value = paging_l012_create_entry_table(
			    (uint64_t)pts);

			page_table_store->block[offset] = new_value;
		} else {
			pts =
			    (union page_table_store*)(old_value &
						      PAGING_L012_TABLE_ADDRESS_MASK);
		}

		int res = paging_insert_or_alloc(paging_manager, pts, level + 1,
						 va, pa, unprivileged_access,
						 read_only);

		return res;
	}

	/*
	 * Level 3 -> points to a 4096K block
	 * */

	uint64_t old_value = page_table_store->block[l3_offset];

	uint64_t new_value = paging_l3_create_entry_block(
	    pa, unprivileged_access, read_only);

	if (old_value == new_value)
		return 0;

	if ((old_value & 0b1) == 1)
		return -1;

	page_table_store->block[l3_offset] = new_value;

	return 0;
}

void paging_init_slab()
{
	void* slab_memory = kalloc(slab_get_needed_size(
	    sizeof(union page_table_store), PAGING_MAX_TABLES));

	slab = slab_create(sizeof(union page_table_store), PAGING_MAX_TABLES,
			   slab_memory);
}

struct slab* paging_get_slab()
{
	return &slab;
}
