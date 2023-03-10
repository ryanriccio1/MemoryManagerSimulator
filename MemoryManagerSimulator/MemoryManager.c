#include <stdlib.h>
#include <stdint.h>
#include <assert.h>


#include "JobManager.h"
#include "Queue.h"
#include "MemoryManager.h"
#include "Constants.h"
#include "Job.h"


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
	setupJob(newJob);
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
		for (size_t idx = 0; idx < VIRTUAL_PAGES; idx++)
		{
			VirtualMemoryPage* page = job->pmt->virtualMemoryPages[idx];
			if (page->valid)
			{
				memoryManager->lruQueue->list->removeAtValue(memoryManager->lruQueue->list, page->physicalMemoryPage, KeepAllocated);
				memoryManager->fifoQueue->list->removeAtValue(memoryManager->fifoQueue->list, page->physicalMemoryPage, KeepAllocated);
				memoryManager->validVirtualPages->removeAtValue(memoryManager->validVirtualPages, page, KeepAllocated);
				memoryManager->freePhysicalPages->append(memoryManager->freePhysicalPages, page->physicalMemoryPage, page->physicalMemoryPage->index);
				page->physicalMemoryPage->virtualMemoryPage = NULL;
			}
		}
		clearJob(job);
		memoryManager->jobManager->jobs->removeAtValue(memoryManager->jobManager->jobs, job, Deallocate);
		return true;
	}
	return false;
}

void* accessJob(MemoryManager* memoryManager, size_t jobId, uint64_t virtualMemoryAddress, ReplacementMethod method)
{
	assert(memoryManager);
	if (virtualMemoryAddress < VIRTUAL_MEMORY_SIZE)
	{
		Job* job = findJob(memoryManager, jobId);
		if (job)
		{
			uint64_t virtualPageIndex = virtualMemoryAddress >> OFFSET_BITS;
			uint64_t offset = virtualMemoryAddress << (sizeof(uint64_t) * 8 - OFFSET_BITS) >> (sizeof(uint64_t) * 8 - OFFSET_BITS);

			// get from pmt array
			VirtualMemoryPage* virtualPage = job->pmt->virtualMemoryPages[virtualPageIndex];
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
		size_t lowestRefCount = SIZE_MAX;
		switch (method)
		{
		case LRU:
			page = memoryManager->lruQueue->dequeue(memoryManager->lruQueue);
			memoryManager->fifoQueue->list->removeAtValue(memoryManager->fifoQueue->list, page, KeepAllocated);
			break;
		case LFU:
			VirtualMemoryPage* virtualPage = NULL;
			VirtualMemoryPage* tmpVirtualPage = NULL;
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
	else
	{
		memoryManager->freePhysicalPages->sortBySize(memoryManager->freePhysicalPages);
		return memoryManager->freePhysicalPages->getByIndex(memoryManager->freePhysicalPages, 0);
	}
	return NULL;
}


MemoryManager* setupMemoryManager(MemoryManager* memoryManager)
{
	assert(memoryManager);
	memoryManager = realloc(memoryManager, sizeof(MemoryManager) + sizeof(PhysicalMemoryPage*) * PHYSICAL_PAGES);
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
	for (size_t idx = 0; idx < PHYSICAL_PAGES; idx++)
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

	for (size_t idx = 0; idx < PHYSICAL_PAGES; idx++)
	{
		free(memoryManager->physicalMemoryPages[idx]);
	}
}
