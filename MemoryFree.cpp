// 
// 
// 

#include "MemoryFree.h"

extern unsigned int __heap_start;
extern void * __brkval;

struct __freelist
{
	size_t sz;
	struct __freelist * nx;
};

extern struct __freelist * __flp;

int freeListSize()
{
	struct __freelist * current;
	int totalsize;
	
	for (current = __flp; current; current = current->nx)
	{
		totalsize += 2;
		totalsize += (int)current->sz;
	}
	return totalsize;
}

int freeMemory()
{
	int free_memory;
	
	if((int)__brkval == 0)
	{
		free_memory = ((int)&free_memory) - ((int)&__heap_start);
	}
	else
	{
		free_memory = ((int)&free_memory) - ((int)__brkval);
		free_memory += freeListSize();
	}
	return free_memory;
}

