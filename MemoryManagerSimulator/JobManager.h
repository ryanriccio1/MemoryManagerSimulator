#pragma once

#include "LinkedList.h"

/**
 * Just stores a list of all the current jobs (maybe more info in future?)
 */
typedef struct JobManager
{
	LinkedList *jobs;
} JobManager;

/**
 * @brief Initialize job manager.
 *
 * @param jobManager Pointer of job manager to setup.
 */
void setupJobManager(JobManager *jobManager);

/**
 * @brief Free resources used by job manager.
 *
 * @param jobManager Job manager to clear.
 */
void clearJobManager(JobManager *jobManager);