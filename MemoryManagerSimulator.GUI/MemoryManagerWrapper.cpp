#include "MemoryManagerWrapper.h"
#include "MemoryManagerGuiContext.h"

MemoryManagerSimulatorWrapper::MemoryManagerSimulatorWrapper()
{
	memoryManager = new MemoryManager;
	memoryManager = setupMemoryManager(memoryManager);
}

MemoryManagerSimulatorWrapper::~MemoryManagerSimulatorWrapper()
{
	delete memoryManager;
}

void MemoryManagerSimulatorWrapper::m_createJob(char* jobName, size_t jobId) const
{
	createJob(memoryManager, jobName, jobId);
}

bool MemoryManagerSimulatorWrapper::m_removeJob(size_t jobId) const
{
	return removeJob(memoryManager, jobId);
}

void* MemoryManagerSimulatorWrapper::m_accessJob(size_t jobId,
	uint64_t virtualMemoryAddress, ReplacementMethod method) const
{
	return accessJob(memoryManager, jobId, virtualMemoryAddress, method);
}
