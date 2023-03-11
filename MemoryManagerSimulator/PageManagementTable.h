#pragma once

#include "VirtualMemoryPage.h"

typedef struct PageManagementTable
{
	size_t N; // some older compilers don't like a struct with a FLA and no other members (cannot malloc something that size might = 0)
	VirtualMemoryPage *virtualMemoryPages[];
} PageManagementTable;

PageManagementTable *setupPageManagementTable(PageManagementTable *pmt);
void clearPageManagementTable(PageManagementTable *pmt);
