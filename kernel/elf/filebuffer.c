#include <elf/filebuffer.h>

struct filebuffer wrapper_buffer(char* buffer)
{
	struct filebuffer fw = {
	    .buffer = buffer,
	};
	return fw;
}

void read_at_position(void* ptr,
		      size_t offset_bytes,
		      size_t size_bytes,
		      int count,
		      struct filebuffer filewrap)
{
	char* src = filewrap.buffer;
	char* char_ptr = (char*)ptr;
	src += offset_bytes;
	size_t bytes = size_bytes * count;
	while (bytes > 0) {
		bytes--;
		*char_ptr = *src;
		char_ptr++;
		src++;
	}
}
