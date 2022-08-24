#ifndef MP_JOB_H
#define MP_JOB_H

#include <paging.h>
#include <stdint.h>

#define MAX_NAME_CHAR 30
#define MAX_JOBS      256

struct job {
	char name[MAX_NAME_CHAR];

	uint64_t x[31];
	uint64_t pc;
	uint64_t sp;

	struct paging_manager* paging;

	struct job* next;
};

void job_init_slab(size_t max_jobs);

/*
 * Inits the slab allocator and creates the first job.
 * */
struct job* job_init_and_create(uint64_t entry,
				uint64_t sp,
				const char* name,
				struct paging_manager* paging);

/*
 * Adds a new job to the chain.
 * */
struct job* job_create(uint64_t entry,
		       uint64_t sp,
		       const char* name,
		       struct paging_manager* paging);

/*
 * Returns the current job.
 * */
struct job* job_get_current();

/*
 * Goes forward in the jobs chain.
 * */
void job_forward();

#endif
