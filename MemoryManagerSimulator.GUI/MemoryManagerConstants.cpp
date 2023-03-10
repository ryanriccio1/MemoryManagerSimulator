#include "MemoryManagerConstants.h"
extern "C"
{
#include "Constants.h"
}

void MemoryManagerConstants::initializeConsts()
{
	initializeDefaultValues();
}

uint64_t* MemoryManagerConstants::m_PAGE_SIZE = &PAGE_SIZE;
uint64_t* MemoryManagerConstants::m_PHYSICAL_MEMORY_SIZE = &PHYSICAL_MEMORY_SIZE;
uint64_t* MemoryManagerConstants::m_VIRTUAL_MEMORY_SIZE = &VIRTUAL_MEMORY_SIZE;	// virtual memory per job

uint64_t* MemoryManagerConstants::m_OFFSET_BITS = &OFFSET_BITS;
uint64_t* MemoryManagerConstants::m_INSTRUCTION_BITS = &INSTRUCTION_BITS;
uint64_t* MemoryManagerConstants::m_PAGE_BITS = &PAGE_BITS;
uint64_t* MemoryManagerConstants::m_VIRTUAL_PAGES = &VIRTUAL_PAGES;
uint64_t* MemoryManagerConstants::m_PHYSICAL_PAGES = &PHYSICAL_PAGES;
