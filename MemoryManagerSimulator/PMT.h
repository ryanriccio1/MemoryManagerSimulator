#pragma once
#include <stdlib.h>
#include <stdio.h>

#include "LinkedList.h"
#include "TLB.h"

//Page Map Table structure for Linked List page and Lookup Cache

typedef struct PMT
{
	LinkedList* pages;
	TLB* TLBCache;
} PMT;