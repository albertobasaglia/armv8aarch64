#include <block.h>

int block_write(struct block* block, const void* ptr, size_t n)
{
	int res = block->write(ptr, n, block->args);
	return res;
}

int block_read(struct block* block, void* ptr, size_t n)
{
	int res = block->read(ptr, n, block->args);
	return res;
}
