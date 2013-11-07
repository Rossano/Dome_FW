//////////////////////////////////////////////////////////////////////////
/// 
///	Include Section
/// 
//////////////////////////////////////////////////////////////////////////

/// Include Nil RTOS
#include <NilRTOS.h>

/// 
///	Project related included files
/// 
#include "board.h"			//	Board related inclusions
#include "encoder.h"		//	Encoder related inclusions
#include "dome.h"			//	Dome control related inclusions
#include "shell.h"			//	Shell related inclusions

/// 
///	Debug inclusions, use this to simulate the encoder with a timer
///	
///	NOT USED!
/// 
#undef DEBUG				//	this inclusions is not used
#ifdef DEBUG
	#include <NilTimer1.h>	//	NilRTOS Timer
	#include "debug.h"		//	Debug related inclusions
#endif // DEBUG

//////////////////////////////////////////////////////////////////////////
///
///	Defines Section
///
//////////////////////////////////////////////////////////////////////////

///		Thread Stacksize
#define STACKSIZE	64			
/////		Activate the simulation of the encoder
//#define ENCODER_SIMULATION

//////////////////////////////////////////////////////////////////////////
///
///	Thread Section
///
//////////////////////////////////////////////////////////////////////////
///
///	This section allow to have visible the thread data structure
///	defined elsewhere
/// 
//////////////////////////////////////////////////////////////////////////

/// 
///	Encoder Thread
/// 
extern void EncoderThread(void * arg);
extern NIL_WORKING_AREA(waEncoderThread, STACKSIZE);
/// 
///	Dome Control Thread
///
extern void DomeThread(void * arg);
extern NIL_WORKING_AREA(waDomeThread, STACKSIZE);
/// 
///	Shell Thread
/// 
#ifdef USE_SHELL_THREAD
	extern void ShellThread(void *arg);
	extern NIL_WORKING_AREA(waShellThread, STACKSIZE);
#endif // USE_SHELL_THREAD
///
///	Encoder Simulation (Debug) Thread
/// 
#if defined(ENCODER_SIMULATION)
	extern void DebugThread(void * arg);
	extern NIL_WORKING_AREA(waDebugThread, STACKSIZE);
#endif // ENCODER_SIMULATION

//////////////////////////////////////////////////////////////////////////
///
///	Thread Table Definition Section
///
//////////////////////////////////////////////////////////////////////////

NIL_THREADS_TABLE_BEGIN()
///	Encoder Simulation Thread
#if defined(ENCODER_SIMULATION)
	NIL_THREADS_TABLE_ENTRY("Debug", DebugThread, NULL, waDebugThread, sizeof(waDebugThread))		
#endif // ENCODER_SIMULATION
///	Shell Thread
#ifdef USE_SHELL_THREAD
	NIL_THREADS_TABLE_ENTRY("Shell", ShellThread, NULL, waShellThread, sizeof(waShellThread))
#endif // USE_SHELL_THREAD
///	Encoder Thread
NIL_THREADS_TABLE_ENTRY("Encoder", EncoderThread, NULL, waEncoderThread, sizeof(waEncoderThread))
///	Dome Control Thread
NIL_THREADS_TABLE_ENTRY("Dome", DomeThread, NULL, waDomeThread, sizeof(waDomeThread))

NIL_THREADS_TABLE_END()

//////////////////////////////////////////////////////////////////////////
///
///	Variable Section
///
//////////////////////////////////////////////////////////////////////////

/// 
///	Variables used for the shell code but defined elsewhere
///
#ifndef USE_SHELL_THREAD
	extern String cmdString;					//        Line Command string buffer
	extern boolean cmdReady;					//        Flag indicating that a command is ready
	extern uint8_t inBufCount;					//        Input buffer char counter
#endif

//////////////////////////////////////////////////////////////////////////
///
///	Code Section
///
//////////////////////////////////////////////////////////////////////////

///<summary>
///	Arduino Setup function entry point
///</summary>
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

///<summary>
///	Arduino loop function entry point
///</summary>
void loop()
{
	//
	// Idle task, read the data from serial port only if NOT USED Shell Thread
	//
	#ifndef USE_SHELL_THREAD	
		char ch;	
		//	Execute the USB-CDC task
		CDC_Task();
		//
		//	If the '\n' is received the flag cmdReady is set and the input string is consumed by the Shell
		//
		if (cmdReady)
		{
			#ifdef DEBUG
				Serial.println("Received -> " + cmdString);
			#endif // DEBUG
		
			char buffer[CMD_STRING_LEN];
			//	Create the char buffer pointer for the shell
			char *buf = (char *)&buffer;
			cmdString.toCharArray(buf, CMD_STRING_LEN);
			//	Execute the Shell task with the data coming for the PC
			ShellTask((void *)ShellCommands, buf);
			//	Print the prompt
			Serial.print(PROMPT);
			//	Reinitialize the input buffer and the flag
			cmdString = "";
			cmdReady = false;
			inBufCount = 0;
		}	
	#endif // USE_SHELL_THREAD		
}
	