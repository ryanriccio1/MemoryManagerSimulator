#pragma once
#include "MemoryManagerConstants.h"

extern "C"
{
#include "MemoryManager.h"
}

class MemoryManagerSimulatorWrapper
{
public:
	MemoryManagerSimulatorWrapper();
	~MemoryManagerSimulatorWrapper();
	void m_createJob(char* jobName, size_t jobId) const;
	bool m_removeJob(size_t jobId) const;
	void* m_accessJob(size_t jobId, uint64_t virtualMemoryAddress, ReplacementMethod method) const;

	MemoryManager* memoryManager;
};