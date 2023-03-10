#pragma once
#include <cstdint>

class MemoryManagerConstants
{
public:
	static void initializeConsts();
	static uint64_t* m_PAGE_SIZE;
	static uint64_t* m_PHYSICAL_MEMORY_SIZE;
	static uint64_t* m_VIRTUAL_MEMORY_SIZE;	// virtual memory per job
		   
	static uint64_t* m_OFFSET_BITS;
	static uint64_t* m_INSTRUCTION_BITS;
	static uint64_t* m_PAGE_BITS;
	static uint64_t* m_VIRTUAL_PAGES;
	static uint64_t* m_PHYSICAL_PAGES;

	
} ;
