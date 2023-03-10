#include <math.h>
#include <stdint.h>

#include "Constants.h"

void initializeDefaultValues()
{
	PAGE_SIZE = 16;
	PHYSICAL_MEMORY_SIZE = 128;
	VIRTUAL_MEMORY_SIZE = 1024;	// virtual memory per job

	OFFSET_BITS = (uint64_t)log2(PAGE_SIZE);
	INSTRUCTION_BITS = (uint64_t)log2(VIRTUAL_MEMORY_SIZE);
	PAGE_BITS = (INSTRUCTION_BITS - OFFSET_BITS);
	VIRTUAL_PAGES = (VIRTUAL_MEMORY_SIZE / PAGE_SIZE);
	PHYSICAL_PAGES = (PHYSICAL_MEMORY_SIZE / PAGE_SIZE);
}