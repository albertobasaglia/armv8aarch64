/*
 *    We need control bits to allow allocating more than 4096 bytes
 *  of memory at the same time (and with the user not noticing it).
 *
 *    There will be a table storing the state of blocks. 1 byte will be
 *  the size of each entry in the table, leaving us space to expand its
 *  functionality in the future. We will need:
 *
 *  - A bit to determine if this is the first block of the allocated region.
 *  - A bit to determine if it is the last block, meaning we iterated all
 *    the blocks.
 *  - A bit that tells us if this block is currently allocated or is free
 *    to give to the user.
 *
 *    Our allocation algorithm will be allocating the first n free block,
 *  supposing an user requested (memory requested DIV 4096bytes) blocks
 *  of memory. This of course is going to fragment the memory. There are
 *  lots of different ways to prevent this.
 * */

#ifndef HEAP_H
#define HEAP_H
#include <stddef.h>

#define HEAP_BLOCK_SIZE_BYTES 4096

#define HEAP_BLOCK_TAKEN      0x80
#define HEAP_BLOCK_FIRST      0x40
#define HEAP_BLOCK_LAST       0x20

/*
 *  We will use this to point to our heap table.
 * */
typedef char* heap_table_t;

struct heap {
	char* heap_start_address;
	heap_table_t heap;
	size_t blocks;
};

/*
 *  Allocate memory from the heap.
 * */
void* heap_alloc(struct heap* heap, size_t size);

/*
 *  Free memory from the heap.
 * */
void heap_free(struct heap* heap, void* ptr);

/*
 *  Create the table
 * */
struct heap heap_createtable(char* heap_start_address,
			       char* table_address,
			       size_t heap_blocks);

/*
 *  Marks the argument passed region as taken.
 *  From and to are intended as [from, to]
 * */
void mark_region_taken(heap_table_t from, heap_table_t to);

/*
 *  Frees a region of memory.
 * */
void mark_region_free(heap_table_t from);

/*
 *  Expected size for the table in bytes.
 *
 *  @param blocks blocks count
 *  @return the size of the table in bytes
 * */
size_t heap_table_size(size_t blocks);

#endif
