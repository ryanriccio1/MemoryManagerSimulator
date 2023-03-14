#pragma once
#include <stddef.h>

#include "VirtualMemoryPage.h"

/**
 * Store information about a physical page.
 */
typedef struct PhysicalMemoryPage
{
	size_t index;								 // index of physical page
	uint64_t physicalAddress;					 // address of physical page
	struct VirtualMemoryPage *virtualMemoryPage; // virtual memory page that is allocated in this physical page
} PhysicalMemoryPage;

/**
 * @brief Initializes page to default values.
 *
 * @param memory Physical page to setup.
 * @param index Index of page being seutp.
 * @param physicalAddress (FAKE) Address that physical page will be located at.
 */
void setupPhysicalMemoryPage(PhysicalMemoryPage *memory, size_t index, uint64_t physicalAddress);