#include "VirtualMemoryPage.h"

#include <assert.h>

void setupVirtualMemoryPage(VirtualMemoryPage* page, size_t index)
{
	assert(page);

	page->index = index;
	page->refCount = 0;
	page->valid = false;
	//page->modified = false;
	//page->referenced = false;
	page->physicalMemoryPage = NULL;
}
