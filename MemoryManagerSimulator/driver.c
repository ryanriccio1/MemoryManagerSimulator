#include "Constants.h"
#include "MemoryManager.h"
#include <stdio.h>
#include <stdlib.h>


int main()
{
	initializeDefaultValues();
	printf("PAGE SIZE: %lld\n", PAGE_SIZE);
	printf("PHYSICAL MEMORY: %lld\n", PHYSICAL_MEMORY_SIZE);
	printf("VIRUTAL MEMORY: %lld\n", VIRTUAL_MEMORY_SIZE);

	printf("OFFSET BITS: %lld\n", OFFSET_BITS);
	printf("PAGE BITS: %lld\n", PAGE_BITS);
	printf("INSTRUCTION BITS: %lld\n", INSTRUCTION_BITS);
	printf("VIRTUAL PAGES: %lld\n", VIRTUAL_PAGES);
	printf("PHYSICAL PAGES: %lld\n", PHYSICAL_PAGES);
	
	MemoryManager* memMgr = malloc(sizeof(MemoryManager));
	memMgr = setupMemoryManager(memMgr);
	createJob(memMgr, "test", 1);
	createJob(memMgr, "test2", 123);

	for (size_t idx = 0; idx < 2000; idx++)
	{
		accessJob(memMgr, 1, idx * 30 % 1024, FIFO);
		accessJob(memMgr, 123, idx * 236 % 1024, LRU);
	}

	removeJob(memMgr, 1);

	printf("\nVVP:\n");
	memMgr->validVirtualPages->print(memMgr->validVirtualPages);
	printf("\nFPP:\n");
	memMgr->freePhysicalPages->print(memMgr->freePhysicalPages);
	printf("\nFIFO:\n");
	memMgr->fifoQueue->list->print(memMgr->fifoQueue->list);
	printf("\nLRU:\n");
	memMgr->lruQueue->list->print(memMgr->lruQueue->list);

	cleanupMemoryManager(memMgr);
}
