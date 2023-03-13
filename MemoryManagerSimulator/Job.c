#include <assert.h>
#include <stdlib.h>

#include "Job.h"
#include "VirtualMemoryPage.h"
#include "MemoryManager.h"


Job* setupJob(MemoryManager* memoryManager, Job* job)
{
	assert(job);
	assert(memoryManager);

	job = realloc(job, sizeof(Job) + sizeof(VirtualMemoryPage*) * memoryManager->VIRTUAL_PAGES);

	for (size_t idx = 0; idx < memoryManager->VIRTUAL_PAGES; idx++)
	{
		job->virtualMemoryPages[idx] = malloc(sizeof(VirtualMemoryPage));
		setupVirtualMemoryPage(job->virtualMemoryPages[idx], idx);
	}
	return job;
}
void clearJob(MemoryManager* memoryManager, Job* job)
{
	assert(job);
	assert(memoryManager);

	job->name = NULL;
	for (size_t idx = 0; idx < memoryManager->VIRTUAL_PAGES; idx++)
	{
		free(job->virtualMemoryPages[idx]);
	}
}
