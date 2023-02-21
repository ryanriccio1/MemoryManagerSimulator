#pragma once

#include <stdlib.h>
#include <stdio.h>
#include "PMT.h"


typedef struct Job
{
	char* name;
	size_t id;
	PMT* pageManagementTable;
} Job;