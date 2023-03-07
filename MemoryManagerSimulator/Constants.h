#pragma once
#include <math.h>

#define PAGE_SIZE				16
#define PHYSICAL_MEMORY_SIZE	128
#define VIRTUAL_MEMORY_SIZE		1024	// virtual memory per job

#define OFFSET_BITS				(uint32_t)log2(PAGE_SIZE)
#define INSTRUCTION_BITS		(uint32_t)log2(VIRTUAL_MEMORY_SIZE)
#define PAGE_BITS				(uint32_t)(INSTRUCTION_BITS - OFFSET_BITS)
#define VIRTUAL_PAGES			(uint32_t)(VIRTUAL_MEMORY_SIZE / PAGE_SIZE)
#define PHYSICAL_PAGES			(uint32_t)(PHYSICAL_MEMORY_SIZE / PAGE_SIZE)

#define TLB_LENGTH				10


//typedef enum State
//{
//	Success = 0,
//	JobDoesNotExist,
//	UnknownFailure
//} State;
//
//State GLOBAL_STATE;
