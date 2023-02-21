#pragma once
#include <stdlib.h>
#include <stdio.h>

#include "Memory.h"
#include "Queue.h"
#include "Constants.h"


typedef struct MemoryManager
{
	Queue* pageQueue;
	Memory physicalMemoryPage[PHYSICAL_PAGES]; 
} MemoryManager;
