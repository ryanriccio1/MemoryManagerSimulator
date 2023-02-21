#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "linkedlist.h"
#include "Constants.h"



typedef struct Memory
{
	size_t index;
	Page* virtualMemoryPage;	
} Memory;