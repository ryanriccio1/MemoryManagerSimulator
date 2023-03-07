#pragma once

#include "LinkedList.h"

typedef struct JobManager
{
	LinkedList* jobs;
} JobManager;

void setupJobManager(JobManager* jobManager);
void clearJobManager(JobManager* jobManager);