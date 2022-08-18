#ifndef PAGING_H
#define PAGING_H

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

uint64_t paging_l012_create_entry_block(uint64_t block_address, int level);

uint64_t paging_l012_create_entry_table(uint64_t table_address);

#endif
