#ifndef FILEBUFFER_H
#define FILEBUFFER_H

#include <stddef.h>
#include <stdint.h>

struct filebuffer {
	char* buffer;
};

void read_at_position(void* ptr,
		      size_t offset_bytes,
		      size_t size_bytes,
		      int count,
		      struct filebuffer filewrap);
#endif
