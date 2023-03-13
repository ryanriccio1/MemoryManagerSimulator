#include <stdio.h>
#include <stdlib.h>

#include "MemoryManager.h"


int main()
{
    MemoryManager* memMgr = malloc(sizeof(MemoryManager));
    memMgr = setupMemoryManager(memMgr, 16, 128, 1024);

    printf("PAGE SIZE: %lld\n", memMgr->PAGE_SIZE);
    printf("PHYSICAL MEMORY: %lld\n", memMgr->PHYSICAL_MEMORY_SIZE);
    printf("VIRUTAL MEMORY: %lld\n", memMgr->VIRTUAL_MEMORY_SIZE);

    printf("OFFSET BITS: %lld\n", memMgr->OFFSET_BITS);
    printf("PAGE BITS: %lld\n", memMgr->PAGE_BITS);
    printf("INSTRUCTION BITS: %lld\n", memMgr->INSTRUCTION_BITS);
    printf("VIRTUAL PAGES: %lld\n", memMgr->VIRTUAL_PAGES);
    printf("PHYSICAL PAGES: %lld\n", memMgr->PHYSICAL_PAGES);

    
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
