#pragma once
#include <stdlib.h>
#include <stdio.h>

#include "Job.h"

typedef struct JobManager
{
	LinkedList* jobs;
} JobManager;

State createJob(JobManager* jobManager, char* jobName, size_t jobId);
State removeJob(JobManager* jobManager, size_t jobId);
State access(JobManager* jobManager, PMT* PageManagementTablePtr);