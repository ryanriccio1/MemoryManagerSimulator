#pragma once
#include "VirtualMemoryPage.h"


typedef struct Job
{
	char* name;
	size_t id;
	VirtualMemoryPage* virtualMemoryPages[];
} Job;

Job* setupJob(Job* job);
void clearJob(Job* job);