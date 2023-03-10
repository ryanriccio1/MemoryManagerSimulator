#include <stdlib.h>

#include "Job.h"

#include <assert.h>

void setupJob(Job* job)
{
	assert(job);

	job->pmt = malloc(sizeof(PageManagementTable));
	job->pmt = setupPageManagementTable(job->pmt);
}
void clearJob(Job* job)
{
	assert(job);

	job->name = NULL;

	clearPageManagementTable(job->pmt);
}
