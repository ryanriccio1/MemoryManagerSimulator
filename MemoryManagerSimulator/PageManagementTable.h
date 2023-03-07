#pragma once

#include "Constants.h"
#include "Queue.h"
#include "VirtualMemoryPage.h"


typedef struct PageManagementTable
{
	VirtualMemoryPage* virtualMemoryPages[VIRTUAL_PAGES];
} PageManagementTable;

void setupPageManagementTable(PageManagementTable* pmt);
void clearPageManagementTable(PageManagementTable* pmt);

