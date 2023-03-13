#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

#include "JobManager.h"
#include "Job.h"
#include "Queue.h"
#include "MemoryManager.h"


Job* createJob(MemoryManager* memoryManager, char* jobName, size_t jobId)
{
	assert(memoryManager);

	for (size_t idx = 0; idx < memoryManager->jobManager->jobs->length; idx++)
	{
		Job* currentJob = memoryManager->jobManager->jobs->getByIndex(memoryManager->jobManager->jobs, idx);
		if (currentJob->id == jobId)
			return NULL;
	}
	Job* newJob = malloc(sizeof(Job));
	newJob = setupJob(memoryManager, newJob);
	newJob->id = jobId;
	newJob->name = jobName;
	memoryManager->jobManager->jobs->append(memoryManager->jobManager->jobs, newJob, sizeof(Job));
	return newJob;
}

bool removeJob(MemoryManager* memoryManager, size_t jobId)
{
	assert(memoryManager);

	Job* job = findJob(memoryManager, jobId);
	if (job)
	{
		for (size_t idx = 0; idx < memoryManager->VIRTUAL_PAGES; idx++)
		{
			VirtualMemoryPage* page = job->virtualMemoryPages[idx];
			if (page->valid)
			{
				memoryManager->lruQueue->list->removeAtValue(memoryManager->lruQueue->list, page->physicalMemoryPage, KeepAllocated);
				memoryManager->fifoQueue->list->removeAtValue(memoryManager->fifoQueue->list, page->physicalMemoryPage, KeepAllocated);
				memoryManager->validVirtualPages->removeAtValue(memoryManager->validVirtualPages, page, KeepAllocated);
				memoryManager->freePhysicalPages->append(memoryManager->freePhysicalPages, page->physicalMemoryPage, page->physicalMemoryPage->index);
				page->physicalMemoryPage->virtualMemoryPage = NULL;
			}
		}
		clearJob(memoryManager, job);
		memoryManager->jobManager->jobs->removeAtValue(memoryManager->jobManager->jobs, job, Deallocate);
		return true;
	}
	return false;
}

void* accessJob(MemoryManager* memoryManager, size_t jobId, uint64_t virtualMemoryAddress, ReplacementMethod method)
{
	assert(memoryManager);
	if (virtualMemoryAddress < memoryManager->VIRTUAL_MEMORY_SIZE)
	{
		Job* job = findJob(memoryManager, jobId);
		if (job)
		{
			uint64_t virtualPageIndex = virtualMemoryAddress >> memoryManager->OFFSET_BITS;
			uint64_t offset = virtualMemoryAddress << (sizeof(uint64_t) * 8 - memoryManager->OFFSET_BITS) >> (sizeof(uint64_t) * 8 - memoryManager->OFFSET_BITS);

			// get from pmt array
			VirtualMemoryPage* virtualPage = job->virtualMemoryPages[virtualPageIndex];
			virtualPage->refCount++;


			if (virtualPage->physicalMemoryPage)
			{
				memoryManager->lruQueue->list->removeAtValue(memoryManager->lruQueue->list, virtualPage->physicalMemoryPage, KeepAllocated);
				memoryManager->lruQueue->enqueue(memoryManager->lruQueue, virtualPage->physicalMemoryPage, sizeof(VirtualMemoryPage));
				return (void*)(virtualPage->physicalMemoryPage->physicalAddress + offset);
			}
			else
			{
				PhysicalMemoryPage* physicalPage = getFreePage(memoryManager, method);
				virtualPage->physicalMemoryPage = physicalPage;
				physicalPage->virtualMemoryPage = virtualPage;
				virtualPage->valid = true;

				memoryManager->fifoQueue->enqueue(memoryManager->fifoQueue, physicalPage, sizeof(VirtualMemoryPage));
				memoryManager->lruQueue->enqueue(memoryManager->lruQueue, physicalPage, sizeof(VirtualMemoryPage));

				memoryManager->validVirtualPages->append(memoryManager->validVirtualPages, virtualPage, sizeof(VirtualMemoryPage));
				memoryManager->freePhysicalPages->removeAtValue(memoryManager->freePhysicalPages, physicalPage, KeepAllocated);

				return (void*)(virtualPage->physicalMemoryPage->physicalAddress + offset);
			}
		}
	}
	return NULL;
}

Job* findJob(MemoryManager* memoryManager, size_t jobId)
{
	assert(memoryManager);

	for (size_t idx = 0; idx < memoryManager->jobManager->jobs->length; idx++)
	{
		Job* job = memoryManager->jobManager->jobs->getByIndex(memoryManager->jobManager->jobs, idx);
		if (job->id == jobId)
		{
			return job;
		}
	}
	return NULL;
}

PhysicalMemoryPage* getFreePage(MemoryManager* memoryManager, ReplacementMethod method)
{
	assert(memoryManager);
	if (memoryManager->freePhysicalPages->isEmpty)
	{
		PhysicalMemoryPage* page;
		VirtualMemoryPage* virtualPage = NULL;
		VirtualMemoryPage* tmpVirtualPage = NULL;
		size_t lowestRefCount = SIZE_MAX;
		switch (method)
		{
		case LRU:
			page = memoryManager->lruQueue->dequeue(memoryManager->lruQueue);
			memoryManager->fifoQueue->list->removeAtValue(memoryManager->fifoQueue->list, page, KeepAllocated);
			break;
		case LFU:
			for (size_t idx = 0; idx < memoryManager->validVirtualPages->length; idx++)
			{
				tmpVirtualPage = memoryManager->validVirtualPages->getByIndex(memoryManager->validVirtualPages, idx);
				if (tmpVirtualPage->refCount < lowestRefCount)
				{
					virtualPage = tmpVirtualPage;
					lowestRefCount = virtualPage->refCount;
				}
			}
			page = virtualPage->physicalMemoryPage;
			memoryManager->fifoQueue->list->removeAtValue(memoryManager->fifoQueue->list, page, KeepAllocated);
			memoryManager->lruQueue->list->removeAtValue(memoryManager->lruQueue->list, page, KeepAllocated);
			break;
		case FIFO:
		default:
			page = memoryManager->fifoQueue->dequeue(memoryManager->fifoQueue);
			memoryManager->lruQueue->list->removeAtValue(memoryManager->lruQueue->list, page, KeepAllocated);
			break;
		}
		memoryManager->validVirtualPages->removeAtValue(memoryManager->validVirtualPages, page->virtualMemoryPage, KeepAllocated);
		memoryManager->freePhysicalPages->append(memoryManager->freePhysicalPages, page, page->index);

		page->virtualMemoryPage->physicalMemoryPage = NULL;
		page->virtualMemoryPage->valid = false;
		page->virtualMemoryPage->refCount = 0;
		page->virtualMemoryPage = NULL;
		return page;
	}
	memoryManager->freePhysicalPages->sortBySize(memoryManager->freePhysicalPages);
	return memoryManager->freePhysicalPages->getByIndex(memoryManager->freePhysicalPages, 0);

}


MemoryManager* setupMemoryManager(MemoryManager* memoryManager, uint64_t PAGE_SIZE, uint64_t PHYSICAL_MEMORY_SIZE,
																uint64_t VIRTUAL_MEMORY_SIZE)
{
	assert(memoryManager);
	memoryManager->PAGE_SIZE = PAGE_SIZE;
	memoryManager->PHYSICAL_MEMORY_SIZE = PHYSICAL_MEMORY_SIZE;
	memoryManager->VIRTUAL_MEMORY_SIZE = VIRTUAL_MEMORY_SIZE;	// virtual memory per job

	memoryManager->OFFSET_BITS = uint64log2(PAGE_SIZE);
	memoryManager->INSTRUCTION_BITS = uint64log2(VIRTUAL_MEMORY_SIZE);
	memoryManager->PAGE_BITS = memoryManager->INSTRUCTION_BITS - memoryManager->OFFSET_BITS;
	memoryManager->VIRTUAL_PAGES = VIRTUAL_MEMORY_SIZE / PAGE_SIZE;
	memoryManager->PHYSICAL_PAGES = PHYSICAL_MEMORY_SIZE / PAGE_SIZE;

	memoryManager = realloc(memoryManager, sizeof(MemoryManager) + sizeof(PhysicalMemoryPage*) * memoryManager->PHYSICAL_PAGES);
	assert(memoryManager);

	memoryManager->jobManager = malloc(sizeof(JobManager));
	memoryManager->lruQueue = malloc(sizeof(Queue));
	memoryManager->fifoQueue = malloc(sizeof(Queue));
	memoryManager->validVirtualPages = malloc(sizeof(LinkedList));
	memoryManager->freePhysicalPages = malloc(sizeof(LinkedList));
	
	setupJobManager(memoryManager->jobManager);
	setupQueue(memoryManager->lruQueue, Pointer);
	setupQueue(memoryManager->fifoQueue, Pointer);
	setupLinkedList(memoryManager->validVirtualPages, Pointer);
	setupLinkedList(memoryManager->freePhysicalPages, Pointer);

	uint64_t nextAddress = 0x0000;
	for (size_t idx = 0; idx < memoryManager->PHYSICAL_PAGES; idx++)
	{
		memoryManager->physicalMemoryPages[idx] = malloc(sizeof(PhysicalMemoryPage));
		setupPhysicalMemoryPage(memoryManager->physicalMemoryPages[idx], idx, nextAddress);
		memoryManager->freePhysicalPages->append(memoryManager->freePhysicalPages, memoryManager->physicalMemoryPages[idx], idx);
		nextAddress += PAGE_SIZE;
	}
	return memoryManager;
}

void cleanupMemoryManager(MemoryManager* memoryManager)
{
	assert(memoryManager);

	clearJobManager(memoryManager->jobManager);

	free(memoryManager->jobManager);
	free(memoryManager->lruQueue);
	free(memoryManager->fifoQueue);
	free(memoryManager->validVirtualPages);
	free(memoryManager->freePhysicalPages);

	for (size_t idx = 0; idx < memoryManager->PHYSICAL_PAGES; idx++)
	{
		free(memoryManager->physicalMemoryPages[idx]);
	}
}

uint64_t uint64log2(uint64_t input)
{
	uint64_t exp = 0;

	while (input >>= 1)
	{
		exp++;
	}
	return exp;
}