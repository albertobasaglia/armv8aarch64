#include <mp/job.h>
#include <slab.h>
#include <sysutils.h>

#include <stdint.h>
#include <string.h>

/*
 * Slab allocator for the jobs.
 * */
static struct slab job_slab;

/*
 * Holds the current chain.
 * */
static struct job* current_job;

struct job* job_init_and_create(uint64_t entry,
				uint64_t sp,
				const char* name,
				struct paging_manager* paging)
{

	job_init_slab(MAX_JOBS);
	struct job* job = slab_allocate(&job_slab);

	current_job = job;

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

struct job* job_create(uint64_t entry,
		       uint64_t sp,
		       const char* name,
		       struct paging_manager* paging)
{
	struct job* job = slab_allocate(&job_slab);

	job->pc = entry;
	job->sp = sp;
	job->paging = paging;

	// Adds the newly created job after the current one
	struct job* save = current_job->next;
	current_job->next = job;
	job->next = save;

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

struct job* job_get_current()
{
	return current_job;
}

void job_forward()
{
	current_job = current_job->next;
}