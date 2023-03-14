#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

#include "JobManager.h"
#include "Job.h"
#include "Queue.h"
#include "MemoryManager.h"

Job *createJob(MemoryManager *memoryManager, char *jobName, size_t jobId)
{
	assert(memoryManager);

	// loop through all existing jobs
	for (size_t idx = 0; idx < memoryManager->jobManager->jobs->length; idx++)
	{
		// make sure that the job id does not exist in all of our jobs
		const Job *currentJob = memoryManager->jobManager->jobs->getByIndex(memoryManager->jobManager->jobs, idx);
		if (currentJob->id == jobId)
			return NULL;
	}

	// allocate our job, set it up, add to jobManager
	Job *newJob = malloc(sizeof(Job));
	newJob = setupJob(memoryManager, newJob);
	newJob->id = jobId;
	newJob->name = jobName;
	memoryManager->jobManager->jobs->append(memoryManager->jobManager->jobs, newJob, sizeof(Job));
	return newJob;
}

bool removeJob(MemoryManager *memoryManager, size_t jobId)
{
	assert(memoryManager);

	// see if the job id exists
	Job *job = findJob(memoryManager, jobId);
	if (job)	// if the jobId exists
	{
		// loop clear all of the virtual pages from any data management structures
		for (size_t idx = 0; idx < memoryManager->VIRTUAL_PAGES; idx++)
		{
			// get the current virtual page
			VirtualMemoryPage *page = job->virtualMemoryPages[idx];
			// only remove virtual page if it is currently in physical memory
			if (page->valid)
			{
				// on removal, do not deallocate any of the pages, we are just setting them to null, we do not want to clear them yet, that is clearJob()'s job
				memoryManager->lruQueue->list->removeAtValue(memoryManager->lruQueue->list, page->physicalMemoryPage, KeepAllocated);
				memoryManager->fifoQueue->list->removeAtValue(memoryManager->fifoQueue->list, page->physicalMemoryPage, KeepAllocated);
				memoryManager->validVirtualPages->removeAtValue(memoryManager->validVirtualPages, page, KeepAllocated);
				memoryManager->freePhysicalPages->append(memoryManager->freePhysicalPages, page->physicalMemoryPage, page->physicalMemoryPage->index);	// add to the list of free physical pages
				page->physicalMemoryPage->virtualMemoryPage = NULL;	// unlink the physical memory to our virtual page
			}
		}
		// clears job memory and removes it from job manager
		clearJob(memoryManager, job);
		memoryManager->jobManager->jobs->removeAtValue(memoryManager->jobManager->jobs, job, Deallocate);
		return true;
	}
	return false;
}

void *accessJob(MemoryManager *memoryManager, size_t jobId, uint64_t virtualMemoryAddress, ReplacementMethod method)
{
	assert(memoryManager);
	// only access the virtual memory if it is a valid address
	if (virtualMemoryAddress < memoryManager->VIRTUAL_MEMORY_SIZE)
	{
		// make sure the job exists by ID
		const Job *job = findJob(memoryManager, jobId);
		if (job)
		{	// use the higher order bits to get the index of the virtual page within the job
			const uint64_t virtualPageIndex = virtualMemoryAddress >> memoryManager->OFFSET_BITS;
			// use the lower order bits to get the offset within the virtual page of the address we want
			// creates bitmask
			const uint64_t offset = virtualMemoryAddress & ((1 << memoryManager->OFFSET_BITS) - 1);

			// find the virtual page we are accessing and increase its reference
			VirtualMemoryPage *virtualPage = job->virtualMemoryPages[virtualPageIndex];
			virtualPage->refCount++;

			// if this page is already allocated in a physical page
			if (virtualPage->physicalMemoryPage)
			{	// remove it from the lrqQueue and add it back to the top
				memoryManager->lruQueue->list->removeAtValue(memoryManager->lruQueue->list, virtualPage->physicalMemoryPage, KeepAllocated);
				memoryManager->lruQueue->enqueue(memoryManager->lruQueue, virtualPage->physicalMemoryPage, sizeof(VirtualMemoryPage));
				// get the pointer to physical memory
				return (void *)(virtualPage->physicalMemoryPage->physicalAddress + offset);
			}
			else
			{	// find a free place to allocate our virtual page
				PhysicalMemoryPage *physicalPage = getFreePage(memoryManager, method);

				// link our virtual and physical pages
				virtualPage->physicalMemoryPage = physicalPage;
				physicalPage->virtualMemoryPage = virtualPage;
				virtualPage->valid = true;

				// add to the queues that keep track of the order data is accessed
				memoryManager->fifoQueue->enqueue(memoryManager->fifoQueue, physicalPage, sizeof(VirtualMemoryPage));
				memoryManager->lruQueue->enqueue(memoryManager->lruQueue, physicalPage, sizeof(VirtualMemoryPage));

				// keep track of all of the virtual pages that are in physical memory
				memoryManager->validVirtualPages->append(memoryManager->validVirtualPages, virtualPage, sizeof(VirtualMemoryPage));

				// keep remove the physical page from the free memory list
				memoryManager->freePhysicalPages->removeAtValue(memoryManager->freePhysicalPages, physicalPage, KeepAllocated);

				// get the pointer to physical memory
				return (void *)(virtualPage->physicalMemoryPage->physicalAddress + offset);
			}
		}
	}
	return NULL;
}

Job *findJob(const MemoryManager *memoryManager, size_t jobId)
{
	assert(memoryManager);

	// loop through all jobs
	for (size_t idx = 0; idx < memoryManager->jobManager->jobs->length; idx++)
	{
		// if our ID matches the ID we're searching for return a pointer to the job
		Job *job = memoryManager->jobManager->jobs->getByIndex(memoryManager->jobManager->jobs, idx);
		if (job->id == jobId)
		{
			return job;
		}
	}
	return NULL;
}

PhysicalMemoryPage *getFreePage(const MemoryManager *memoryManager, ReplacementMethod method)
{
	assert(memoryManager);
	// if we have no free pages, calculate where we can free one
	if (memoryManager->freePhysicalPages->isEmpty)
	{
		PhysicalMemoryPage *page;
		VirtualMemoryPage *virtualPage = NULL;
		VirtualMemoryPage *tmpVirtualPage = NULL;
		size_t lowestRefCount = SIZE_MAX;
		// we have multiple methods we can use to get a free page
		switch (method)
		{
		case LRU:
			// get the least recently used page, remove it from the other queue
			page = memoryManager->lruQueue->dequeue(memoryManager->lruQueue);
			memoryManager->fifoQueue->list->removeAtValue(memoryManager->fifoQueue->list, page, KeepAllocated);
			break;
		case LFU:
			// calculate which page has the lowest reference count
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
			// free from other queues
			memoryManager->fifoQueue->list->removeAtValue(memoryManager->fifoQueue->list, page, KeepAllocated);
			memoryManager->lruQueue->list->removeAtValue(memoryManager->lruQueue->list, page, KeepAllocated);
			break;
		case FIFO:
		default:
			// get the page that was allocated first and remove it from other queue as well
			page = memoryManager->fifoQueue->dequeue(memoryManager->fifoQueue);
			memoryManager->lruQueue->list->removeAtValue(memoryManager->lruQueue->list, page, KeepAllocated);
			break;
		}
		// in all cases, we want to remove the page from the list of valid pages and add the physical page to the list of free memory pages
		memoryManager->validVirtualPages->removeAtValue(memoryManager->validVirtualPages, page->virtualMemoryPage, KeepAllocated);
		memoryManager->freePhysicalPages->append(memoryManager->freePhysicalPages, page, page->index);

		// after removing the page, set its values accordingly
		page->virtualMemoryPage->physicalMemoryPage = NULL;
		page->virtualMemoryPage->valid = false;
		page->virtualMemoryPage->refCount = 0;
		page->virtualMemoryPage = NULL;
		return page;
	}
	// make sure all pages are in order then return the first free page
	memoryManager->freePhysicalPages->sortBySize(memoryManager->freePhysicalPages);
	return memoryManager->freePhysicalPages->getByIndex(memoryManager->freePhysicalPages, 0);
}

MemoryManager *setupMemoryManager(MemoryManager *memoryManager, uint64_t PAGE_SIZE, uint64_t PHYSICAL_MEMORY_SIZE,
								  uint64_t VIRTUAL_MEMORY_SIZE)
{
	assert(memoryManager);
	memoryManager->PAGE_SIZE = PAGE_SIZE;
	memoryManager->PHYSICAL_MEMORY_SIZE = PHYSICAL_MEMORY_SIZE;
	memoryManager->VIRTUAL_MEMORY_SIZE = VIRTUAL_MEMORY_SIZE; // virtual memory per job

	// calculate all other values needed
	memoryManager->OFFSET_BITS = uint64log2(PAGE_SIZE);
	memoryManager->INSTRUCTION_BITS = uint64log2(VIRTUAL_MEMORY_SIZE);
	memoryManager->PAGE_BITS = memoryManager->INSTRUCTION_BITS - memoryManager->OFFSET_BITS;
	memoryManager->VIRTUAL_PAGES = VIRTUAL_MEMORY_SIZE / PAGE_SIZE;
	memoryManager->PHYSICAL_PAGES = PHYSICAL_MEMORY_SIZE / PAGE_SIZE;

	// make space for Flexible Length Array in the MemoryManager struct
	memoryManager = realloc(memoryManager, sizeof(MemoryManager) + sizeof(PhysicalMemoryPage *) * memoryManager->PHYSICAL_PAGES);
	assert(memoryManager);

	// allocate all of our data structures
	memoryManager->jobManager = malloc(sizeof(JobManager));
	memoryManager->lruQueue = malloc(sizeof(Queue));
	memoryManager->fifoQueue = malloc(sizeof(Queue));
	memoryManager->validVirtualPages = malloc(sizeof(LinkedList));
	memoryManager->freePhysicalPages = malloc(sizeof(LinkedList));

	// setup data structures
	setupJobManager(memoryManager->jobManager);
	setupQueue(memoryManager->lruQueue, Pointer);
	setupQueue(memoryManager->fifoQueue, Pointer);
	setupLinkedList(memoryManager->validVirtualPages, Pointer);
	setupLinkedList(memoryManager->freePhysicalPages, Pointer);

	// calculate the physical address of each physical page
	uint64_t nextAddress = 0x0000;
	for (size_t idx = 0; idx < memoryManager->PHYSICAL_PAGES; idx++)
	{
		memoryManager->physicalMemoryPages[idx] = malloc(sizeof(PhysicalMemoryPage));
		setupPhysicalMemoryPage(memoryManager->physicalMemoryPages[idx], idx, nextAddress);
		memoryManager->freePhysicalPages->append(memoryManager->freePhysicalPages, memoryManager->physicalMemoryPages[idx], idx);
		nextAddress += PAGE_SIZE;
	}
	// since we realloc'ed we need to return a pointer to where we realloc'ed to
	return memoryManager;
}

void cleanupMemoryManager(MemoryManager *memoryManager)
{
	assert(memoryManager);

	// len of jobmanager->jobs changes mid loop, so store it to var before
	const size_t numJobs = memoryManager->jobManager->jobs->length;	// remove all jobs
	for (size_t jobIdx = 0; jobIdx < numJobs; jobIdx++)
	{
		const size_t jobId = ((Job*)memoryManager->jobManager->jobs->getByIndex(memoryManager->jobManager->jobs, 0))->id;
		removeJob(memoryManager, jobId);
	}

	// clear all data structures
	cleanQueue(memoryManager->lruQueue);
	cleanQueue(memoryManager->fifoQueue);
	clear(memoryManager->validVirtualPages);
	clear(memoryManager->freePhysicalPages);
	clearJobManager(memoryManager->jobManager);

	// free all memory
	free(memoryManager->jobManager);
	free(memoryManager->lruQueue);
	free(memoryManager->fifoQueue);
	free(memoryManager->validVirtualPages);
	free(memoryManager->freePhysicalPages);
}

uint64_t uint64log2(uint64_t input)
{
	// rather than use math.h log2, use ours since its
	// faster for ints only
	uint64_t exp = 0;

	while (input >>= 1)
	{
		exp++;
	}
	return exp;
}