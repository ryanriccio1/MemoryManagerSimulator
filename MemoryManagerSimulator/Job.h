#pragma once
#include "PageManagementTable.h"


typedef struct Job
{
	char* name;
	size_t id;
	PageManagementTable* pmt;
} Job;

void setupJob(Job* job);
void clearJob(Job* job);