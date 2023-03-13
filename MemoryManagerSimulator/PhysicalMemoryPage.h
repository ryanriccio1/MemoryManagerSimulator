#pragma once
#include <stddef.h>

#include "VirtualMemoryPage.h"

typedef struct PhysicalMemoryPage
{
	size_t index;
	uint64_t physicalAddress;
	struct VirtualMemoryPage *virtualMemoryPage;
} PhysicalMemoryPage;

void setupPhysicalMemoryPage(PhysicalMemoryPage *memory, size_t index, uint64_t physicalAddress);