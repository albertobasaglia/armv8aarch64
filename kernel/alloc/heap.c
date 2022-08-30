#include <log.h>
#include <heap.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct heap heap_createtable(char* heap_start_address,
			     char* table_address,
			     size_t heap_blocks)
{
	heap_table_t table = table_address;
	for (int i = 0; i < heap_blocks; i++)
		*table = 0;

	struct heap heap = {
	    .heap_start_address = heap_start_address,
	    .heap = table_address,
	    .blocks = heap_blocks,
	    .free_blocks = heap_blocks,
	};
	return heap;
}

void* heap_alloc(struct heap* heap, size_t size)
{
	if (size == 0)
		return NULL;
	// How many blocks we will need
	heap_table_t block = heap->heap;
	size_t alloc_blocks = size / HEAP_BLOCK_SIZE_BYTES;
	if (size % HEAP_BLOCK_SIZE_BYTES != 0)
		alloc_blocks++;

	// Search for the first free space

	heap_table_t last_block = block + heap->blocks;
	while (block < last_block) {
		while (*block & HEAP_BLOCK_TAKEN && block < last_block) {
			block++;
		}
		heap_table_t start = block;

		if (block == last_block) {
			goto err;
		}

		// An empty block is found
		// Do we have enough space to allocate?

		size_t blocks_found = 0;

		while (blocks_found < alloc_blocks && block < last_block &&
		       !(*block & HEAP_BLOCK_TAKEN)) {
			block++;
			blocks_found++;
		}

		if (blocks_found == alloc_blocks) {
			// Success, now we need to mark the blocks
			mark_region_taken(start, block - 1);
			heap->free_blocks -= alloc_blocks;
			return ((void*)heap->heap_start_address +
				(int)(start - heap->heap) *
				    HEAP_BLOCK_SIZE_BYTES);
		}

		if (block == last_block) {
			goto err;
		}
	}

err:
	return 0;
}

void mark_region_taken(heap_table_t from, heap_table_t to)
{
	*from |= HEAP_BLOCK_FIRST;
	*to |= HEAP_BLOCK_LAST;
	while (from <= to) {
		*from |= HEAP_BLOCK_TAKEN;
		from++;
	}
}

void heap_free(struct heap* heap, void* ptr)
{
	int offset = ptr - ((void*)heap->heap_start_address);
	heap_table_t start = heap->heap + offset / HEAP_BLOCK_SIZE_BYTES;
	// We need to be given a "first" block to free memory
	if (!(*start & HEAP_BLOCK_FIRST))
		return;
	int freed_blocks = mark_region_free(start);
	/* klogf("Freed %q blocks", freed_blocks); */
	heap->free_blocks += freed_blocks;
}

size_t mark_region_free(heap_table_t from)
{
	size_t freed = 1;
	while (!(*from & HEAP_BLOCK_LAST)) {
		*from = 0;
		from++;
		freed++;
	}
	*from = 0;
	return freed;
}

size_t heap_table_size(size_t blocks)
{
	return blocks * sizeof(heap_table_t);
}

size_t heap_get_free_blocks(struct heap* heap)
{
	return heap->free_blocks;
}
