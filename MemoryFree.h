// MemoryFree.c.h

#ifndef _MEMORYFREE_h
#define _MEMORYFREE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

//int freeMemory();
int freeMemory();

#endif	// _MEMORYFREE_h


