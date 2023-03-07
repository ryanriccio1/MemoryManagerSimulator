#include <assert.h>
#include <stdlib.h>

#include "PageManagementTable.h"
#include "VirtualMemoryPage.h"



void setupPageManagementTable(PageManagementTable* pmt)
{
	assert(pmt);

	for (size_t idx = 0; idx < VIRTUAL_PAGES; idx++)
	{
		pmt->virtualMemoryPages[idx] = malloc(sizeof(VirtualMemoryPage));
		setupVirtualMemoryPage(pmt->virtualMemoryPages[idx], idx);
	}
}

void clearPageManagementTable(PageManagementTable* pmt)
{
	assert(pmt);
	for (size_t idx = 0; idx < VIRTUAL_PAGES; idx++)
	{
		free(pmt->virtualMemoryPages[idx]);
	}
}