#pragma once

#include "VirtualMemoryPage.h"


typedef struct PageManagementTable
{
	VirtualMemoryPage* virtualMemoryPages[];
} PageManagementTable;

PageManagementTable* setupPageManagementTable(PageManagementTable* pmt);
void clearPageManagementTable(PageManagementTable* pmt);

