#ifndef JOB_H
#define JOB_H

#include "paging.h"
#include <stdint.h>

#define MAX_NAME_CHAR 30

struct job {
	char name[MAX_NAME_CHAR];

	uint64_t x[31];
	uint64_t pc;
	uint64_t sp;

	struct paging_manager* paging;

	struct job* next;
};

struct job job_create(uint64_t entry,
		      uint64_t sp,
		      const char* name,
		      struct paging_manager* paging);

#endif
