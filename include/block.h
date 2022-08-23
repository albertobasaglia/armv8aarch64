#ifndef BLOCK_H
#define BLOCK_H

#include <stdbool.h>
#include <stddef.h>

/*
 *    ptr: pointer to memory containing the data (to write from, to read to)
 *      n: sector to read
 * */
typedef int(block_read_fnp)(void* ptr, size_t n, void* args);
typedef int(block_write_fnp)(const void* ptr, size_t n, void* args);

struct block {
	block_read_fnp* read;
	block_write_fnp* write;
	size_t sector_size;
	void* args;
	bool read_only;
};

int block_write(struct block* block, const void* ptr, size_t n);

int block_read(struct block* block, void* ptr, size_t n);

#endif
