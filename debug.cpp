// 
// 
// 

#if defined(DEBUG)

#include <NilTimer1.h>
#include "debug.h"

NIL_WORKING_AREA(waDebugThread, 64);
NIL_THREAD(DebugThread, arg)
{
	nilTimer1Start(DEBUG_TIMER_INTERVAL_US);
	while(TRUE)
	{
		nilTimer1Wait();
		nilSemSignal(&DebugSem);
	}
}

#endif