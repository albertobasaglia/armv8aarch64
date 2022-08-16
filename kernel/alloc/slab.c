#include <slab.h>

#include <stddef.h>
#include <stdint.h>

size_t slab_get_needed_size(size_t object_size, size_t max_store)
{
	// an uint8_t contains 8 bits
	size_t header = slab_get_header_size(object_size, max_store);
	size_t objects = object_size * max_store;
	return header + objects;
}

int slab_align_to_object(size_t object_size, size_t size)
{
	int count = 0;
	while (size > 0) {
		size -= object_size;
		count++;
	}
	return object_size * count;
}

size_t slab_get_header_size(size_t object_size, size_t max_store)
{
	int offset = max_store;
	int quot = 0;
	while (offset >= 8) {
		quot++;
		offset -= 8;
	}
	int header_bytes = offset == 0 ? quot : quot + 1;
	return slab_align_to_object(object_size, header_bytes);
}

inline char slab_header_reset_value()
{
	return 0;
}

Slab slab_create(size_t object_size, size_t max_store, void* starting_address)
{
	size_t total_size = slab_get_needed_size(object_size, max_store);
	size_t header_size = slab_get_header_size(object_size, max_store);
	Slab slab = {
	    .starting_address = starting_address,
	    .max_store = max_store,
	    .header_size = header_size,
	    .object_size = object_size,
	};
	// set all header bytes to the default value
	char default_val = slab_header_reset_value();
	char* ptr = (char*)starting_address;
	char* last = ptr + header_size;
	while (ptr < last) {
		*ptr = default_val;
		ptr++;
	}
	return slab;
}

int slab_get_header_bit(Slab* slab, int n)
{
	uint8_t* header_byte = (uint8_t*)slab->starting_address;
	int offset = n % 8;
	int quot = n / 8;

	return (header_byte[quot] & (1 << offset)) != 0 ? 1 : 0;
}

void slab_set_header_bit(Slab* slab, int n, int v)
{
	uint8_t* header_byte = (uint8_t*)slab->starting_address;
	int offset = n % 8;
	int quot = n / 8;

	header_byte[quot] &= ~(1 << offset);
	header_byte[quot] |= (v << offset);
}

size_t slab_get_next_slot(Slab* slab)
{
	size_t count = slab->max_store - 1;
	while (count >= 0) {
		int bit = slab_get_header_bit(slab, count);
		if (bit == 0) {
			return count;
		}
		count--;
	}
	return -1;
}

void* slab_allocate(Slab* slab)
{
	int slot = slab_get_next_slot(slab);
	if (slot == -1) {
		return NULL;
	}

	slab_set_header_bit(slab, slot, 1);

	uint32_t slot_position = slab->header_size;
	slot_position += slab->object_size * slot;

	return slab->starting_address + slot_position;
}

int slab_free(Slab* slab, void* ptr)
{
	size_t offset = (size_t)ptr;
	offset -= (size_t)slab->starting_address;
	offset -= slab->header_size;

	int slot = 0;
	while (offset >= slab->object_size) {
		offset -= slab->object_size;
		slot++;
	}

	if (offset != 0) {
		// something is wrong!
		return 0;
	}

	int bit = slab_get_header_bit(slab, slot);
	if (bit == 1) {
		slab_set_header_bit(slab, slot, 0);
		return 1;
	}
	return 0;
}

size_t slab_get_size(Slab* slab)
{
	return slab_get_needed_size(slab->object_size, slab->max_store);
}
