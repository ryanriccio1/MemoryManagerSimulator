#include <stdlib.h>

#include "Job.h"

#include <assert.h>

#include "Constants.h"
#include "VirtualMemoryPage.h"

Job* setupJob(Job* job)
{
	assert(job);
	job = realloc(job, sizeof(Job) + sizeof(VirtualMemoryPage*) * VIRTUAL_PAGES);

	for (size_t idx = 0; idx < VIRTUAL_PAGES; idx++)
	{
		job->virtualMemoryPages[idx] = malloc(sizeof(VirtualMemoryPage));
		setupVirtualMemoryPage(job->virtualMemoryPages[idx], idx);
	}
	return job;
}
void clearJob(Job* job)
{
	assert(job);

	job->name = NULL;
	for (size_t idx = 0; idx < VIRTUAL_PAGES; idx++)
	{
		free(job->virtualMemoryPages[idx]);
	}
}
