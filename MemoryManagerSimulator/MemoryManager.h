#pragma once

#include <stdint.h>

#include "Job.h"
#include "PhysicalMemoryPage.h"
#include "Queue.h"
#include "JobManager.h"

typedef enum ReplacementMethod
{
	LRU,
	LFU,
	FIFO
} ReplacementMethod;

typedef struct MemoryManager
{
	JobManager* jobManager;
	Queue* lruQueue;
	Queue* fifoQueue;
	LinkedList* validVirtualPages;
	LinkedList* freePhysicalPages;
	PhysicalMemoryPage* physicalMemoryPages[PHYSICAL_PAGES];
} MemoryManager;

void createJob(MemoryManager* memoryManager, char* jobName, size_t jobId);
bool removeJob(MemoryManager* memoryManager, size_t jobId);
void* accessJob(MemoryManager* memoryManager, size_t jobId, uint64_t virtualMemoryAddress, ReplacementMethod method);
Job* findJob(MemoryManager* memoryManager, size_t jobId);
PhysicalMemoryPage* getFreePage(MemoryManager* memoryManager, ReplacementMethod method);

void setupMemoryManager(MemoryManager* memoryManager);
void cleanupMemoryManager(MemoryManager* memoryManager);

