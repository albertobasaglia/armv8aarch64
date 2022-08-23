#include <paging.h>
#include <slab.h>
#include <stdbool.h>
#include <stdint.h>

uint64_t paging_l012_create_entry_block(uint64_t block_address,
					int level,
					bool unprivileged_access,
					bool read_only)
{
	if (level <= 0 || level > 2)
		return 0;

	uint64_t entry = PAGING_L012_VALID;
	entry |= block_address;

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

uint64_t paging_create_entry_invalid()
{
	return 0;
}

void paging_set_ttbr0(uint64_t table_address)
{
	asm volatile("msr TTBR0_EL1, %0" ::"r"(table_address));
}

void paging_set_tcr()
{
	uint64_t tcr_ips = 0b10;
	tcr_ips = tcr_ips << 32;
	tcr_ips |= 32;
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

void paging_manager_apply(struct paging_manager* paging_manager)
{
	paging_set_ttbr0((uint64_t)paging_manager->l1);
	paging_set_tcr();
	paging_enable();
	asm volatile("tlbi vmalle1");
}

void paging_manager_init(struct paging_manager* paging_manager,
			 struct slab* pages_allocator)
{
	paging_manager->pages_allocator = pages_allocator;

	struct page_table_store* l1 = slab_allocate(
	    paging_manager->pages_allocator);

	for (int i = 0; i < PAGING_ENTRIES_PER_TABLE; i++) {
		l1->entries[i] = paging_create_entry_invalid();
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
	paging_manager->l1->entries[gb_offset] = paging_l012_create_entry_block(
	    pa, 1, unprivileged_access, read_only);
}
