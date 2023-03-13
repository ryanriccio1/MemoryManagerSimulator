#pragma once
#include "MemoryManager.h"
#include "VirtualMemoryPage.h"


typedef struct Job
{
	char* name;
	size_t id;
	VirtualMemoryPage* virtualMemoryPages[];
} Job;

Job* setupJob(struct MemoryManager* memoryManager, Job* job);
void clearJob(struct MemoryManager* memoryManager, Job* job);