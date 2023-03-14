#pragma once
extern "C"
{
#include "MemoryManager.h"
}

/**
 * Wrapper for MemoryManager C library
 */
class MemoryManagerSimulatorWrapper
{
public:
	/**
	 * @brief Construct a new Memory Manager Simulator Wrapper object.
	 *
	 * @param pageSize Size of each individual page.
	 * @param physicalMemorySize Size of shared physical memory.
	 * @param virtualMemorySize Size of virtual memory for each job.
	 */
	MemoryManagerSimulatorWrapper(uint64_t pageSize, uint64_t physicalMemorySize, uint64_t virtualMemorySize);
	~MemoryManagerSimulatorWrapper();

	/**
	 * @brief Wrapper for createJob().
	 *
	 * @param jobName Name of job to be created.
	 * @param jobId ID of job being created.
	 */
	void m_createJob(char *jobName, size_t jobId) const;
	/**
	 * @brief Wrapper for removeJob().
	 *
	 * @param jobId ID of job to remove.
	 * @return True if job removed.
	 * @return False if not removed.
	 */
	bool m_removeJob(size_t jobId) const;
	/**
	 * @brief Wrapper for accessJob().
	 *
	 * @param jobId ID of job to access.
	 * @param virtualMemoryAddress Virtual memory address to access.
	 * @param method Method of freeing if pages need to be swapped.
	 * @return (Fake) pointer to a physical memory address.
	 */
	void *m_accessJob(size_t jobId, uint64_t virtualMemoryAddress, ReplacementMethod method) const;
	/**
	 * @brief Wrapper for uint64log2(). Perform int log2 calculation.
	 *
	 * @param input Variable to perform calculation on.
	 * @return Int representation of log2(input).
	 */
	static uint64_t m_uint64log2(uint64_t input);

	MemoryManager *memoryManager;

	uint64_t PAGE_SIZE;
	uint64_t PHYSICAL_MEMORY_SIZE;
	uint64_t VIRTUAL_MEMORY_SIZE;

	uint64_t OFFSET_BITS;
	uint64_t PAGE_BITS;
	uint64_t INSTRUCTION_BITS;
	uint64_t PHYSICAL_PAGES;
	uint64_t VIRTUAL_PAGES;
};