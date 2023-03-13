#include <assert.h>
#include <stdlib.h>

#include "JobManager.h"


void setupJobManager(JobManager* jobManager)
{
	assert(jobManager);
	jobManager->jobs = malloc(sizeof(LinkedList));
	setupLinkedList(jobManager->jobs, Pointer);
}

void clearJobManager(JobManager* jobManager)
{
	assert(jobManager);
	jobManager->jobs->clear(jobManager->jobs);
	free(jobManager->jobs);
}