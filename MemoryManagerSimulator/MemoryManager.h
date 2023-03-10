#pragma once

#include <stdint.h>

#include "Job.h"
#include "PhysicalMemoryPage.h"
#include "Queue.h"
#include "JobManager.h"

typedef enum ReplacementMethod
{
	LRU = 0,
	LFU = 1,
	FIFO = 2
} ReplacementMethod;

typedef struct MemoryManager
{
	JobManager* jobManager;
	Queue* lruQueue;
	Queue* fifoQueue;
	LinkedList* validVirtualPages;
	LinkedList* freePhysicalPages;
	PhysicalMemoryPage* physicalMemoryPages[];
} MemoryManager;

Job* createJob(MemoryManager* memoryManager, char* jobName, size_t jobId);
bool removeJob(MemoryManager* memoryManager, size_t jobId);
void* accessJob(MemoryManager* memoryManager, size_t jobId, uint64_t virtualMemoryAddress, ReplacementMethod method);
Job* findJob(MemoryManager* memoryManager, size_t jobId);
PhysicalMemoryPage* getFreePage(MemoryManager* memoryManager, ReplacementMethod method);

MemoryManager* setupMemoryManager(MemoryManager* memoryManager);
void cleanupMemoryManager(MemoryManager* memoryManager);

