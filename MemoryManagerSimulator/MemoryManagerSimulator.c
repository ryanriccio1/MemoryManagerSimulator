#include "Constants.h"
#include <stdio.h>
#include <stdlib.h>


int main()
{
	printf("PAGE SIZE: %d\n", PAGE_SIZE);
	printf("PHYSICAL MEMORY: %d\n", PHYSICAL_MEMORY_SIZE);
	printf("VIRUTAL MEMORY: %d\n", VIRTUAL_MEMORY_SIZE);

	printf("OFFSET BITS: %d\n", OFFSET_BITS);
	printf("PAGE BITS: %d\n", PAGE_BITS);
	printf("INSTRUCTION BITS: %d\n", INSTRUCTION_BITS);
	printf("VIRTUAL PAGES: %d\n", VIRTUAL_PAGES);
	printf("PHYSICAL PAGES: %d\n", PHYSICAL_PAGES);
	
	printf("TLB LEN: %d\n", TLB_LENGTH);
}
