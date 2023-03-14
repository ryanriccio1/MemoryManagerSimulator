#include <assert.h>
#include <stdlib.h>

#include "JobManager.h"

void setupJobManager(JobManager *jobManager)
{
	assert(jobManager);
	// setup job list
	jobManager->jobs = malloc(sizeof(LinkedList));
	setupLinkedList(jobManager->jobs, Pointer);
}

void clearJobManager(JobManager *jobManager)
{
	assert(jobManager);
	// free job list
	jobManager->jobs->clear(jobManager->jobs);
	free(jobManager->jobs);
}