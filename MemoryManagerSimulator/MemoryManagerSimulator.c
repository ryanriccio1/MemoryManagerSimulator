#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>


int main()
{
	LINKED_LIST* list = (LINKED_LIST*)malloc(sizeof(LINKED_LIST));
	setupLinkedList(list);
	printf("DO WE HAVE A LINKED LIST? %p", list);
	printf("I THINK WE DO!");
}