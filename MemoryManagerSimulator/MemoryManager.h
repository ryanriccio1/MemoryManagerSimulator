#pragma once

#include <stdint.h>

#include "Job.h"
#include "PhysicalMemoryPage.h"
#include "Queue.h"
#include "JobManager.h"

/**
 * Determine how pages should be freed.
 */
typedef enum ReplacementMethod
{
	LRU = 0,
	LFU = 1,
	FIFO = 2
} ReplacementMethod;

/**
 * Stores the main state of the Memory manager.
 */
typedef struct MemoryManager
{
	JobManager *jobManager;		   // Keeps track of allocated jobs
	Queue *lruQueue;			   // Least Recently Used Queue
	Queue *fifoQueue;			   // First In First Out Queue
	LinkedList *validVirtualPages; // Keeps track of virtual pages that are currently allocated
	LinkedList *freePhysicalPages; // Keeps track of open physical pages

	uint64_t PAGE_SIZE;
	uint64_t PHYSICAL_MEMORY_SIZE; // amount of shared physical memory
	uint64_t VIRTUAL_MEMORY_SIZE;  // amount of virtual memory per job

	uint64_t OFFSET_BITS;	   // amount of bits needed to calculate offset within page
	uint64_t PAGE_BITS;		   // amount of bits needed to calculate index of virtual page
	uint64_t INSTRUCTION_BITS; // total number of bits needed to store a virtual address
	uint64_t PHYSICAL_PAGES;   // total number of physical pages
	uint64_t VIRTUAL_PAGES;	   // total number of virtual pages

	PhysicalMemoryPage *physicalMemoryPages[]; // all of the shared physical pages
} MemoryManager;

/**
 * @brief Register job with Memory Manager.
 *
 * @param memoryManager MemoryManager to modify.
 * @param jobName Name of new job.
 * @param jobId ID of new job.
 * @return Pointer to the job created. NULL if failed.
 */
struct Job *createJob(MemoryManager *memoryManager, char *jobName, size_t jobId);

/**
 * @brief Removes job from memory manager.
 *
 * @param memoryManager MemoryManager to modify.
 * @param jobId ID of job to remove
 * @return false if it can't be removed.
 * @return true if it can be located and removed.
 */
bool removeJob(MemoryManager *memoryManager, size_t jobId);

/**
 * @brief Performs address calculation to insert new job into physical memory. Will free up physical memory if needed.
 *
 * @param memoryManager MemoryManager to access.
 * @param jobId ID of job to access.
 * @param virtualMemoryAddress Location in virtual memory that is being accessed.
 * @param method FIFO, LRU, LFU.
 * @return The physical addressed calculated. NULL if failed.
 */
void *accessJob(MemoryManager *memoryManager, size_t jobId, uint64_t virtualMemoryAddress, ReplacementMethod method);

/**
 * @brief Searches through Job Manager to locate job.
 *
 * @param memoryManager MemoryManager to search.
 * @param jobId ID of job to find.
 * @return Pointer to the found job, NULL if not found.
 */
struct Job *findJob(MemoryManager *memoryManager, size_t jobId);

/**
 * @brief Will return first free page or free a page using a selected algorithm.
 *
 * @param memoryManager MemoryManager to find free page in.
 * @param method FIFO, LRU, LFU.
 * @return Pointer to the physical page we freed.
 */
PhysicalMemoryPage *getFreePage(MemoryManager *memoryManager, ReplacementMethod method);

/**
 * @brief Sets up memory manager.
 *
 * @param memoryManager MemoryManager to setup.
 * @param PAGE_SIZE Size of each page.
 * @param PHYSICAL_MEMORY_SIZE Total shared physical memory.
 * @param VIRTUAL_MEMORY_SIZE Virtual memory per job.
 * @return A pointer to the reallocated memory manager.
 */
MemoryManager *setupMemoryManager(MemoryManager *memoryManager, uint64_t PAGE_SIZE, uint64_t PHYSICAL_MEMORY_SIZE,
								  uint64_t VIRTUAL_MEMORY_SIZE);

/**
 * @brief Frees resources used by memory manager.
 *
 * @param memoryManager MemoryManager to clear.
 */
void cleanupMemoryManager(MemoryManager *memoryManager);

/**
 * @brief Calculate Log2 of an int, returning it in int form.
 *
 * @param input Value to perform calculation on.
 * @return Int representation of the log2(input).
 */
uint64_t uint64log2(uint64_t input);
