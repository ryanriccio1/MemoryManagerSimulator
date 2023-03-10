#pragma once
#include <stdint.h>

uint64_t PAGE_SIZE;
uint64_t PHYSICAL_MEMORY_SIZE;
uint64_t VIRTUAL_MEMORY_SIZE;	// virtual memory per job

uint64_t OFFSET_BITS;
uint64_t INSTRUCTION_BITS;
uint64_t PAGE_BITS;
uint64_t VIRTUAL_PAGES;
uint64_t PHYSICAL_PAGES;

void initializeDefaultValues();
