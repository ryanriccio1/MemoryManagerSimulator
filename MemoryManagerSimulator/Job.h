#pragma once
#include "MemoryManager.h"
#include "VirtualMemoryPage.h"

/**
 * Stores data about a give job.
 */
typedef struct Job
{
	char *name;								 // name of Job.
	size_t id;								 // ID of Job.
	VirtualMemoryPage *virtualMemoryPages[]; // Virtual memory pages used by Job.
} Job;

/**
 * @brief Setup job.
 *
 * @param memoryManager Pointer to memory manager where job is located.
 * @param job Job being setup.
 * @returns Pointer to job that was setup. (MUST BE REASSIGNED SINCE IT IS REALLOC'ed)
 */
Job *setupJob(const struct MemoryManager *memoryManager, Job *job);

/**
 * @brief Clear resources used by job.
 *
 * @param memoryManager Pointer to memory manager where job is located.
 * @param job Job being cleared.
 */
void clearJob(const struct MemoryManager *memoryManager, Job *job);