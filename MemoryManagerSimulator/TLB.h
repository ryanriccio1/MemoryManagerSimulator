#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "Page.h"
#include "Memory.h"
#include "LinkedList.h"
#include "Constants.h"

//Looks up entry in physical page and vice versa  
typedef struct TLBEntry
{
	Page* virtualPage;
	Memory* physicalPage;
} TLBEntry;

typedef struct TLB
{
	size_t length;
	TLBEntry items[TLB_LENGTH];
} TLB;

bool IsEntryValid = 0;
State LookupEntry(TLBEntry* virtualPage,TLBEntry* physicalPage);
