#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/**
 * Store information about a virtual page.
 */
typedef struct VirtualMemoryPage
{
	size_t index;	 // index within Job of virtual memory page
	size_t refCount; // amount of times this page has been accessed in its current lifecycle
	bool valid;		 // true if the page is currently in physical memory
	// bool modified;	// we have no way to decipher if its modified or not since we don't have real memory
	// bool referenced;	// we don't care if its referenced or not because it will not be allocated until its
	// referenced and it wont be dereferenced until we need its space or the job is removed
	struct PhysicalMemoryPage *physicalMemoryPage;
} VirtualMemoryPage;

/**
 * @brief Initialize a virtual page to its default values.
 *
 * @param page The page to be setup.
 * @param index The index of the page being setup.
 */
void setupVirtualMemoryPage(VirtualMemoryPage *page, size_t index);
