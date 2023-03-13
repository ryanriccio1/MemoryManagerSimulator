#pragma once
extern "C"
{
#include "MemoryManager.h"
}

class MemoryManagerSimulatorWrapper
{
public:
	MemoryManagerSimulatorWrapper(uint64_t pageSize, uint64_t physicalMemorySize, uint64_t virtualMemorySize);
	~MemoryManagerSimulatorWrapper();
	void m_createJob(char* jobName, size_t jobId) const;
	bool m_removeJob(size_t jobId) const;
	void* m_accessJob(size_t jobId, uint64_t virtualMemoryAddress, ReplacementMethod method) const;
	static uint64_t m_uint64log2(uint64_t input);

	MemoryManager* memoryManager;

	uint64_t PAGE_SIZE;
	uint64_t PHYSICAL_MEMORY_SIZE;
	uint64_t VIRTUAL_MEMORY_SIZE;

	uint64_t OFFSET_BITS;
	uint64_t INSTRUCTION_BITS;
	uint64_t PAGE_BITS;
	uint64_t VIRTUAL_PAGES;
	uint64_t PHYSICAL_PAGES;
};