#include <assert.h>

#include "VirtualMemoryPage.h"


void setupVirtualMemoryPage(VirtualMemoryPage* page, size_t index)
{
	assert(page);

	page->index = index;
	page->valid = false;
	page->refCount = 0;
	page->physicalMemoryPage = NULL;
}
