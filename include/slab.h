#ifndef SLAB_H
#define SLAB_H

#include <stddef.h>
#include <stdint.h>

/*
 * The slab structure in memory will also occupy space required by its header
 * All blocks will be aligned to the block size meaning that the header
 * is going to occupy space divisible by the block size
 * */
struct slab{
	void* starting_address;
	size_t max_store;
	size_t object_size; // total allocated zone size
	size_t header_size;
	/*
	 * The header contains bit=0 if the slot is empty.
	 * */
};

/*
 * Creating a slab allocated zone adds an overhead.
 *
 * This returns the total space that will be required.
 *
 * object_size in bytes
 * max_store in objects
 * */
size_t slab_get_needed_size(size_t object_size, size_t max_store);

/*
 * Get total size of the slab allocator
 * */
size_t slab_get_size(struct slab* slab);

/*
 * Get size of the header
 * */
size_t slab_get_header_size(size_t object_size, size_t max_store);

/*
 * This doesn't allocate any memory.
 * It expects the caller to do that.
 * */
struct slab slab_create(size_t object_size, size_t max_store, void* starting_address);

/*
 * Get value used to indicate that a bit represents an empty slab
 * */
char slab_header_reset_value();

/*
 * Returns how many object_size objects are needed to store "size"
 * */
int slab_align_to_object(size_t object_size, size_t size);

/*
 * Returns an index indicating the next empty slab
 * */
size_t slab_get_next_slot(struct slab* slab);

/*
 * Allocate a slab
 * */
void* slab_allocate(struct slab* slab);

/*
 * Free a slab
 * */
int slab_free(struct slab* slab, void* ptr);

#endif
