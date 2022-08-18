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
