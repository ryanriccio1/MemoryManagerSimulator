#include "MemoryManagerWrapper.h"
#include "MemoryManagerGuiContext.h"

MemoryManagerSimulatorWrapper::MemoryManagerSimulatorWrapper(uint64_t pageSize, uint64_t physicalMemorySize, uint64_t virtualMemorySize)
{
	PAGE_SIZE = pageSize;
	PHYSICAL_MEMORY_SIZE = physicalMemorySize;
	VIRTUAL_MEMORY_SIZE = virtualMemorySize;

	OFFSET_BITS = uint64log2(PAGE_SIZE);
	INSTRUCTION_BITS = uint64log2(VIRTUAL_MEMORY_SIZE);
	PAGE_BITS = INSTRUCTION_BITS - OFFSET_BITS;
	PHYSICAL_PAGES = PHYSICAL_MEMORY_SIZE / PAGE_SIZE;
	VIRTUAL_PAGES = VIRTUAL_MEMORY_SIZE / PAGE_SIZE;

	// construct new memory manager
	memoryManager = new MemoryManager;
	memoryManager = setupMemoryManager(memoryManager, PAGE_SIZE, PHYSICAL_MEMORY_SIZE, VIRTUAL_MEMORY_SIZE);
}

MemoryManagerSimulatorWrapper::~MemoryManagerSimulatorWrapper()
{
	// search through all jobs and free all pointers to job names which were allocated by GUI
	for (size_t jobIdx = 0; jobIdx < memoryManager->jobManager->jobs->length; jobIdx++)
	{
		delete static_cast<Job*>(getByIndex(memoryManager->jobManager->jobs, jobIdx))->name;
	}
	// free memoryManager
	cleanupMemoryManager(memoryManager);
	delete memoryManager;
}

void MemoryManagerSimulatorWrapper::m_createJob(char *jobName, size_t jobId) const
{
	createJob(memoryManager, jobName, jobId);
}

bool MemoryManagerSimulatorWrapper::m_removeJob(size_t jobId) const
{
	// if we remove the job early, free the memory allocated by the gui for its name
	delete findJob(memoryManager, jobId)->name;
	return removeJob(memoryManager, jobId);
}

void *MemoryManagerSimulatorWrapper::m_accessJob(size_t jobId,
												 uint64_t virtualMemoryAddress, ReplacementMethod method) const
{
	return accessJob(memoryManager, jobId, virtualMemoryAddress, method);
}

uint64_t MemoryManagerSimulatorWrapper::m_uint64log2(uint64_t input)
{
	return uint64log2(input);
}
