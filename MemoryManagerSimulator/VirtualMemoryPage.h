#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct VirtualMemoryPage
{
	size_t index;
	size_t refCount;
	bool valid;
	// bool modified;	// we have no way to decipher if its modified or not since we don't have real memory
	// bool referenced;	// we don't care if its referenced or not because it will not be allocated until its
						// referenced and it wont be dereferenced until we need its space or the job is removed
	struct PhysicalMemoryPage* physicalMemoryPage;
} VirtualMemoryPage;

void setupVirtualMemoryPage(VirtualMemoryPage* page, size_t index);
