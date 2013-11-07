// 
// 
// 

#if defined(DEBUG)

#include <NilRTOS.h>
#include <NilTimer1.h>

#include "debug.h"

//SEMAPHORE_DECL(DebugSem, 0);

extern void avrPrintf(const char * str);

NIL_WORKING_AREA(waDebugThread, 64);
NIL_THREAD(DebugThread, arg)
{
	#ifdef TIMER_DEBUG
		//Serial.println("Starting Debug Timer");
		avrPrintf("Starting Debug Timer");
	#endif
	nilTimer1Start(DEBUG_TIMER_INTERVAL_US);
	
	while(TRUE)
	{
		nilTimer1Wait();
		nilSemSignal(&DebugSem);
		#ifdef TIMER_DEBUG
			//Serial.println("Tick!\nWaiting Free");
			avrPrintf("Tick");
			avrPrintf("Waiting Free");
		#endif // TIMER_DEBUG				
	}
}

#endif