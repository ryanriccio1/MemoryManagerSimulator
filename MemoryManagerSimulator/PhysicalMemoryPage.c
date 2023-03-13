#include <assert.h>

#include "PhysicalMemoryPage.h"


void setupPhysicalMemoryPage(PhysicalMemoryPage* memory, size_t index, uint64_t physicalAddress)
{
	assert(memory);
	memory->index = index;
	memory->physicalAddress = physicalAddress;
	memory->virtualMemoryPage = NULL;
}
