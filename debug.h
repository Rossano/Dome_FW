// debug.h

//#if defined(DEBUG)

#ifndef _DEBUG_h
#define _DEBUG_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

//#define DEBUG_TIMER_INTERVAL_US		3000000
#define TIMER_DEBUG
#undef TIMER_DEBUG

//extern SEMAPHORE_DECL(DebugSem, 0);
SEMAPHORE_DECL(DebugSem, 0);

#endif

//#endif