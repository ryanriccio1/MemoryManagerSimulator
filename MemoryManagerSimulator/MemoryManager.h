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

	uint64_t PAGE_SIZE;
	uint64_t PHYSICAL_MEMORY_SIZE;
	uint64_t VIRTUAL_MEMORY_SIZE;

	uint64_t OFFSET_BITS;
	uint64_t INSTRUCTION_BITS;
	uint64_t PAGE_BITS;
	uint64_t VIRTUAL_PAGES;
	uint64_t PHYSICAL_PAGES;

	PhysicalMemoryPage* physicalMemoryPages[];
} MemoryManager;

struct Job* createJob(MemoryManager* memoryManager, char* jobName, size_t jobId);
bool removeJob(MemoryManager* memoryManager, size_t jobId);
void* accessJob(MemoryManager* memoryManager, size_t jobId, uint64_t virtualMemoryAddress, ReplacementMethod method);
struct Job* findJob(MemoryManager* memoryManager, size_t jobId);
PhysicalMemoryPage* getFreePage(MemoryManager* memoryManager, ReplacementMethod method);

MemoryManager* setupMemoryManager(MemoryManager* memoryManager, uint64_t PAGE_SIZE, uint64_t PHYSICAL_MEMORY_SIZE, 
																uint64_t VIRTUAL_MEMORY_SIZE);
void cleanupMemoryManager(MemoryManager* memoryManager);

uint64_t uint64log2(uint64_t input);

