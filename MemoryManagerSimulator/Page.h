#pragma once
#include <stdlib.h>
#include <stdio.h>


typedef enum ValidBit
{
	Invalid = 0,
	Valid
} ValidBit;

typedef enum ModifiedBit
{
	NotModified = 0,
	Modified
} ModifiedBit;

typedef enum ReferencedBit
{
	NotReferenced = 0,
	Referenced
} ReferencedBit;

typedef struct Page
{
	size_t index;
	ValidBit valid;
	ModifiedBit modified;
	ReferencedBit referenced;
	Memory* physicalMemoryPage;
} Page;