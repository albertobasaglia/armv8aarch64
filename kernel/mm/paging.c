#include <paging.h>
#include <stdint.h>

uint64_t paging_l012_create_entry_block(uint64_t block_address, int level)
{
	if (level <= 0 || level > 2)
		return 0;

	uint64_t entry = PAGING_L012_VALID;
	entry |= block_address;
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
