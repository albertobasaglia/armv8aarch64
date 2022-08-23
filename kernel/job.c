#include "slab.h"
#include "sysutils.h"
#include <job.h>

#include <stdint.h>
#include <string.h>

struct slab job_slab;

struct job* job_create(uint64_t entry,
		       uint64_t sp,
		       const char* name,
		       struct paging_manager* paging)
{
	struct job* job = slab_allocate(&job_slab);

	job->pc = entry;
	job->sp = sp;
	job->paging = paging;
	job->next = job; // since this will form a chain, being the only element
			 // in it means being the next yourself :)

	strncpy(job->name, name, MAX_NAME_CHAR);

	for (int i = 0; i < 31; i++)
		job->x[i] = 0;

	return job;
}

void job_init_slab(size_t max_jobs)
{
	size_t size = slab_get_needed_size(sizeof(struct job), max_jobs);
	void* slab_zone = kalloc(size);
	job_slab = slab_create(sizeof(struct job), max_jobs, slab_zone);
}
