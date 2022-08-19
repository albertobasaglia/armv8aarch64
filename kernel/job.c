#include <job.h>

#include <stdint.h>
#include <string.h>

struct job job_create(uint64_t entry,
		      uint64_t sp,
		      const char* name,
		      struct paging_manager* paging)
{
	struct job job = {
	    .pc = entry,
	    .sp = sp,
	    .paging = paging,
	    .next = NULL,
	};
	strncpy(job.name, name, MAX_NAME_CHAR);
	for (int i = 0; i < 31; i++)
		job.x[i] = 0;
	return job;
}
