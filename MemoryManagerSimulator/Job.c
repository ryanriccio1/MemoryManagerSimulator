#include <assert.h>
#include <stdlib.h>

#include "Job.h"
#include "VirtualMemoryPage.h"
#include "MemoryManager.h"

Job *setupJob(const MemoryManager *memoryManager, Job *job)
{
	assert(job);
	assert(memoryManager);
	// since virtual memory pages is a flexible length array, we need to realloc the job to be the new size
	// to hold a variable amount of virtual pages
	job = realloc(job, sizeof(Job) + sizeof(VirtualMemoryPage *) * memoryManager->VIRTUAL_PAGES);

	// setup each virtual page
	for (size_t idx = 0; idx < memoryManager->VIRTUAL_PAGES; idx++)
	{
		job->virtualMemoryPages[idx] = malloc(sizeof(VirtualMemoryPage));
		setupVirtualMemoryPage(job->virtualMemoryPages[idx], idx);
	}
	return job;
}
void clearJob(const MemoryManager *memoryManager, Job *job)
{
	assert(job);
	assert(memoryManager);

	// free all memory referenced by this job
	job->name = NULL;
	for (size_t idx = 0; idx < memoryManager->VIRTUAL_PAGES; idx++)
	{
		free(job->virtualMemoryPages[idx]);
	}
}
