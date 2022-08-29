#include "log.h"
#include "paging.h"
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

	struct job* job = job_create(entry, sp, name, paging);

	current_job = job;

	job->next = job; // since this will form a chain, being the only element
			 // in it means being the next yourself :)
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

	for (int i = 0; i < JOB_MAX_FILES; i++)
		job->open_files[i] = NULL;
	job->open_files_count = 0;

	return job;
}

void job_delete(struct job* job)
{
	struct job* last_ptr = current_job;
	struct job* curr_ptr = current_job->next;

	if (last_ptr == curr_ptr) {
		klog("Trying to delete init job");
		while (1)
			;
	}

	while (curr_ptr != job) {
		last_ptr = curr_ptr;
		curr_ptr = curr_ptr->next;
	}

	last_ptr->next = curr_ptr->next;

	kfree(job->paging);

	slab_free(&job_slab, job);
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

int job_add_file(struct job* job, struct inode* inode)
{
	if (job->open_files_count >= JOB_MAX_FILES)
		return -1;

	int fd = 0;
	while (job->open_files[fd] != NULL)
		fd++;

	job->open_files[fd] = inode;
	job->open_files_count++;

	return fd;
}

struct inode* job_get_file(struct job* job, int fd)
{
	if (fd < 0 || fd >= job->open_files_count)
		return NULL;

	return job->open_files[fd];
}

struct paging_manager* job_create_paging()
{
	struct paging_manager* pm = kalloc(sizeof(struct paging_manager));

	paging_manager_init(pm, paging_get_slab());
	paging_manager_map_kernel(
	    pm); // every userprocess has the kernel mapped in!

	// TODO get correct page size
	paging_manager_map_1gb(pm, 0x80000000, 0x80000000, 1, 0);

	return pm;
}
