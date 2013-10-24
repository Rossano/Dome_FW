#include <NilRTOS.h>

#include "encoder.h"
#include "dome.h"
#include "shell.h"

#define STACKSIZE	64

//extern EncoderClass Encoder;

extern void EncoderThread(void * arg);
extern NIL_WORKING_AREA(waEncoderThread, STACKSIZE);

extern void DomeThread(void * arg);
extern NIL_WORKING_AREA(waDomeThread, STACKSIZE);

extern void ShellThread(void *arg);
extern NIL_WORKING_AREA(waShellThread, STACKSIZE);

#ifdef DEBUG
extern void DebugThread(void *arg);
extern NIL_WORKING_AREA(waDebugThread, STACKSIZE);
#endif // DEBUG

NIL_THREADS_TABLE_BEGIN()
NIL_THREADS_TABLE_ENTRY("Encoder", EncoderThread, NULL, waEncoderThread, sizeof(waEncoderThread))
NIL_THREADS_TABLE_ENTRY("Dome", DomeThread, NULL, waDomeThread, sizeof(waDomeThread))
NIL_THREADS_TABLE_ENTRY("Shell", ShellThread, NULL, waShellThread, sizeof(waShellThread))
#ifdef DEBUG
NIL_THREADS_TABLE_ENTRY("Debug", DebugThread, NULL, waDebugThread, sizeof(waDebugThread))
#endif
NIL_THREADS_TABLE_END()

void setup()
{
	
	//
	//        Initialize the board, setting up all the I/O in a known state
	//
	//initBoard();	
	
	//        Initialize the USB link for PC communication
	Serial.begin(9600);
	//
	//        Until the USB CDC is not connected to the PC the yellow LED on Arduino is
	//        blinking, then it will be fix
	//
	while(!Serial)
	{
		digitalWrite(SOL_LED, HIGH);
		delay(300);
		digitalWrite(SOL_LED, LOW);
		delay(300);
	}
		
	//        Highlight the LED in PWM mode to allow the hard fix the light level
	analogWrite(SOL_LED, 32);
	//        Write the prompt on serial link
	avrPrintf(PROMPT);
	//Serial.print(PROMPT);
	//
	//	Starts NilRTOS scheduler
	//
	nilSysBegin();
}

void loop()
{
	//
	// Idle task, Do nothing here
	//
}
