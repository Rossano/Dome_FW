// 
// 
// 

//////////////////////////////////////////////////////////////////////////
///
///	Include Section
///
//////////////////////////////////////////////////////////////////////////

#include <NilRTOS.h>						//	NilRTOS module
#include <NilFIFO.h>						//	RTOS FIFO module

#include "board.h"							//	Dome board definitions inclusion
#include "dome.h"							//	Dome module inclusion
#include "encoder.h"						//	Encoder module inclusion
#include "shell.h"							//	Shell module inclusion

//////////////////////////////////////////////////////////////////////////
///
///	Variable Section
///
//////////////////////////////////////////////////////////////////////////

NilFIFO<int16_t, 2> turnfifo;				//	FIFO to unlock the Dome thread to turn a fix number of step
DomeClass Dome;								//	Dome data structure to control the motors
extern SEMAPHORE_DECL(EncoderCountSem, 0);	//	Semaphore to synchronize Dome thread on encoder pulses

//////////////////////////////////////////////////////////////////////////
///
///	Code Section
///
//////////////////////////////////////////////////////////////////////////

///<summary>
/// Default constructor. It initialize the DomeCLass data structure
///</summary>
DomeClass::DomeClass()
{
	state = NO_TURN;					//	Set as STOPPED
	init();
}

///<summary>
/// Dome HW configuration. It sets up the Arduino I/O for the motor control
///</summary>
void DomeClass::init()
{
	pinMode(turnLeftPin, OUTPUT);
	digitalWrite(turnLeftPin, LOW);
	pinMode(turnRightPin, OUTPUT);
	digitalWrite(turnRightPin, LOW);
}

///<summary>
/// Returns the Dome state
///</summary>
///<return name='DomeStateType'>The Dome rotating state.</return>
DomeStateType DomeClass::getState()
{
	return state;
}

///<summary>
/// Turn the Dome left. This method drive the motor to turn anticlockwise.
///</summary>
///<return name='bool'>TRUE if operation successful, FALSE otherwise.</return>
bool DomeClass::turnLeft()
{
	//	Start turning only if motor is stopped, else command is dropped
	if(state == NO_TURN) 
	{
		state = TURN_LEFT;								//	Change state to turning to the left		
		#ifndef ENCODER_SIMULATION
			digitalWrite(turnLeftPin, HIGH);			//	Activate motor only if real HW is present
		#else
			avrPrintf("Start Turning LEFT\n");			//	If encoder is simulated tag a message to the serial port
			startEncoderTimer();						//	Start the timer to simulate the Encoder
		#endif
		
		#ifdef DEBUG
			avrPrintf("Exiting turning left function\n");
		#endif // DEBUG
		return true;									//	Everything was fine thus return TRUE
	}
	return false;										//	Else retun FALSE
}
///<summary>
/// Turn the Dome right. This method drive the motor to turn clockwise.
///</summary>
///<return name='bool'>TRUE if operation successful, FALSE otherwise.</return>
bool DomeClass::turnRight()
{
	//	Start turning only if motor is stopped, else the command is dropped
	if(state == NO_TURN)
	{
		state = TURN_RIGHT;								//	Change state to turning on the right
		#ifndef ENCODER_SIMULATION
			digitalWrite(turnRightPin, HIGH);			//	Activate motor only if real HW is present
		#else
			avrPrintf("Start turning RIGHT\n");			//	If encoder is simulated tag a message to the serial port
			startEncoderTimer();						//	Start the timer to simulate the Encoder
		#endif
		
		#ifdef DEBUG
			avrPrintf("Exiting turning right function\n");
		#endif // DEBUG
		return true;									//	Everything is fine thus return TRUE
	}
	return false;										//	Else return FALSE
}

///<summary>
/// Method to stop turning the Dome.
///</summary>
void DomeClass::stop()
{
	//	If encoder is not simulated check on which direction the Dome is turning to deactivate
	//	the right Arduino I/O
	#ifndef ENCODER_SIMULATION
		if(state == TURN_LEFT) digitalWrite(turnLeftPin, LOW);
		else if (state == TURN_RIGHT) digitalWrite(turnRightPin, LOW);
	#else
		stopEncoderTimer();								//	Encoder simulated, simply stop the timer
	#endif
	
	state = NO_TURN;									//	Set state to no turning
}

/// <summary>
/// Shell Command to turn the Dome left.
/// Activate the motor to turn anticlockwise the Dome.
/// </summary>
/// <param name="argc">Number of parameters.</param>
/// <param name="argv">A pointer to the Argument list.</param>
void TurnLeft(int argc, char *argv[])
{
	(void) argv;
	//        If there are more than 2 arguments display and error message
	if(argc > 1)
	{
		Usage("turn_left <opt. # of steps>");
		return;
	}
	//        Else  Turning Left
	if(argc == 0)
	{	//	If there are no arguments simply turn left indefinitely
		Dome.turnLeft();									//	Activate the ComeClass method
//		avrPrintf("OK\r\n");								//	Tag an OK to the serial port
	}
	else
	{	//	If there is an argument pass the number of steps to the FIFO
		int16_t *p = turnfifo.waitFree(TIME_INFINITE);		//	Wait a free slot on the FIFO
		#ifdef DEBUG
			avrPrintf("Turning Left of ");
			avrPrintf(*p);
			avrPrintf("\n");
			avrPrintf("argv[0]-> "); avrPrintf(argv[0]);
	//		*p = -atoi(argv[0]);		
		#endif // DEBUG		
		*p = -1 * atoi(argv[0]);							//	Update the FIFO slot with the # of steps to turn (<0 -> left)
		turnfifo.signalData();								//	Signal that there is a new data into the FIFO
	}
	avrPrintf("turn_left OK\r\n");									//	Tag successful operation to the serial port
}

/// <summary>
/// Shell Command to turn the Dome right.
/// Activate the motor to turn clockwise the Dome.
/// </summary>
/// <param name="argc">Number of parameters.</param>
/// <param name="argv">A pointer to the Argument list.</param>
void TurnRight(int argc, char *argv[])
{
	(void) argv;
	//        If there are more than 2 arguments display and error message
	if(argc > 1)
	{
		Usage("turn_right <opt. # of steps>");
		return;
	}
	//        Else  Turning Right
	if(argc == 0)
	{	//	if there are no arguments simply turn right indefinitely
		Dome.turnRight();									//	Activate the Dome Method to turn right
//		avrPrintf("OK\r\n");								
	}
	else
	{	//	If there is an argument pass the number of steps to the FIFO
		int16_t *p = turnfifo.waitFree(TIME_INFINITE);		//	Wait a free slot in the FIFO
		*p = atoi(argv[0]);									//	Update the FIFO slot with the # of steps
		#ifdef DEBUG
			avrPrintf("Turning Right of ");
			avrPrintf(*p);
			avrPrintf("\n");
		#endif
		turnfifo.signalData();								//	Signal that there is a new data into the FIFO
	}
	avrPrintf("turn_right OK\r\n");									//	Tag successful operation to the serial port
}

/// <summary>
/// Shell Command to stop turning the Dome.
/// Deactivate the motor that is actually turning the Dome.
/// </summary>
/// <param name="argc">Number of parameters.</param>
/// <param name="argv">A pointer to the Argument list.</param>
void Stop(int argc, char *argv[])
{
	(void) argv;
	//        If there are arguments display and error message
	if(argc > 0)
	{
		Usage("stop");
		return;
	}
	//        Else stop the Turning Dome
	Dome.stop();											//	Activate the method to stop turning
	avrPrintf("stop OK\r\n");									//	Tag the successful operation on the serial port
}

/// Wrap up  command to return the Dome state.
/// Dummy wrap up to access and simplify access to the Dome state information.
/// </summary>
/// <return name="DomeStateType">Dome Turning State.</return>
DomeStateType getDomeState()
{
	return Dome.getState();
}

/// <summary>
/// Shell Command to read the Dome turning state.
/// This command checks the passed arguments and returns on terminal the
/// actual encoder absolute position.
/// </summary>
/// <param name="argc">Number of passed parameters</param>
/// <param name="argv">List of parameter</param>
void getState(int argc, char *argv[])
{
	(void)argv;
	//	if there are arguments plots an error message
	if(argc > 0)
	{
		Usage("get_state");
		return;
	}
	//	checks the state and plots it to the serial port accordingly
	DomeStateType foo = getDomeState();
	if(foo == NO_TURN) 
	{
		avrPrintf("\r\nstop\r\nget_state OK\r\n");
	}
	else if(foo == TURN_LEFT)
	{
		avrPrintf("\r\nturn_left\r\nget_state OK\r\n");
	}		
	else if(foo == TURN_RIGHT)
	{
		avrPrintf("\r\nturn_right\r\nget_state OK\r\n");
	}
	else
	{
		avrPrintf("\r\nget_state Error: no valid state\r\n");
	}	
}

/// <summary>
/// Shell Command to configure the Dome mechanical gear.
/// This command checks the passed arguments and returns on terminal the
/// actual encoder absolute position.
/// </summary>
/// <param name="argc">Number of passed parameters</param>
/// <param name="argv">List of parameter</param>
void gearCfg(int argc, char *argv[])
{
	(void)argv;
	//	if there are less than 0 or more than 2 arguments print an error message
	//	as well as the actual gear dome configuration
	if((argc > 2) || (argv ==0))
	{
		Usage("gear_config <Encoder Pulse Number> [<gear multiplication ratio>]\nActual values:\nEncoder Pulse number = ");
		avrPrintf(Encoder.encoderResolution);
		avrPrintf("\nGear Ratio = ");
		avrPrintf(int(Encoder.gearRatio));
		return;
	}
	//	Try to convert the first argument and assign it to the encoder resolution variable
	int foo=atoi(argv[1]);
	Encoder.encoderResolution = foo;
	//	Try to convert the second argument if it exists
	if(argc==2)
	{
		float ffoo = atof(argv[2]);
		Encoder.gearRatio = ffoo;
		Encoder.encoderMaxCount = Encoder.encoderResolution * Encoder.gearRatio;		
	}
	avrPrintf("\r\ngearCfg OK\r\n");
}


//////////////////////////////////////////////////////////////////////////
///
///	Thread Section
///
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
///	Dome Thread
///	Thread to synchronize the Thread turning on a given number of 
///	steps. The thread is locked on the FIFO awaiting the number 
///	of steps to turn.
//////////////////////////////////////////////////////////////////////////
NIL_WORKING_AREA(waDomeThread, STACKSIZE);
NIL_THREAD(DomeThread, arg)
{
	while(TRUE)
	{
		int16_t *p = turnfifo.waitData(TIME_INFINITE);			//	Lock the thread until the # of steps to tuen is received
		#if 1 //(defined DEBUG)
			avrPrintf("Using Dome Thread\nReceived data is: ");
			avrPrintf(*p);
			avrPrintf(CR);
		#endif
		uint32_t posInitial = Encoder.Position();				//	Get Encoder initial position
		uint32_t posFinal;										//	Encoder final position
		#ifdef DEBUG
			avrPrintf("Dome Initial Position: ");
			avrPrintf(posInitial);
			avrPrintf(CR);
		#endif // DEBUG		
		int16_t foo = *p;										//	dummy variable
		
		if (foo < 0)
		//if(*p > 0)
		{
			//
			//	Turn Left : carry out final position to reach
			//
			if(posInitial + foo > MAX_COUNT-1) posFinal = posInitial + foo - MAX_COUNT;
			else posFinal = posInitial + foo;
			//if(posInitial + *p > MAX_COUNT-1) posFinal = posInitial + *p - MAX_COUNT;
			//else posFinal = posInitial + *p;
		}
		else
		{
			//
			//	Turn Right : carry out final position to reach
			//
			if(posInitial < foo) posFinal = MAX_COUNT + posInitial + foo;
			else posFinal = posInitial + foo;
			//if(posInitial < *p) posFinal = MAX_COUNT + posInitial + *p;
			//else posFinal = posInitial + *p;
		}
		
		avrPrintf("Dome counts = ");
		avrPrintf(foo);
		avrPrintf(CR);
		
		int16_t count = 0;										//	Steps to count
		Encoder.MultiActivate = true;							//	Activate a flag to activate semaphore
		
		if (foo > 0)
		{	
			//
			//	Turning Right
			//
			avrPrintf("Start turning Right\n");
			if (!Dome.turnRight())								//	Start turning right
			{
				avrPrintf("Error: turnRight command\n");
				break;
			}
			avrPrintf("Entering while\n");
			avrPrintf("foo -> ");
			avrPrintf(foo);
			avrPrintf(CR);
			//	Cycle to count the given # of steps
			//while (count <= foo)
			for (int i=0; i<foo; i++)
			{	avrPrintf("Count");
				nilSemWait(&EncoderCountSem);					//	Lock thread until encoder pulse
				count++;										//	increase the counter
				avrPrintf("Count\n");							//	Tag a message to the serial port
				// Rearm the semaphore
//				nilSemSignal(&EncoderCountSem);
			}
			Dome.stop();										//	Once counted the given # of steps stop turning			
			avrPrintf("Stop turning Right\nOK\n");
		}
		else if (foo < 0)
		{
			//
			//	Turning Left
			//
			avrPrintf("Start turning Left\n");
			if (!Dome.turnLeft())								//	Start turning left
			{
				avrPrintf("Error: turnLeft command\n");
				break;
			}
			uint8_t end = -1 * foo;								//	Define counter limit (positive)
			//	Cycle to count the given # of steps
			while (count <= end)//-foo)
			{
				nilSemWait(&EncoderCountSem);					//	Lock thread until encoder pulse
				count++;										//	increase the counter
				avrPrintf("Count\n");							//	Tag a message to the serial port
				//	Rearm the semahaore
//				nilSemSignal(&EncoderCountSem);
			}
			Dome.stop();										//	Once counted the given # of steps stop turning			
			avrPrintf("Stop Turning Left\nOK\n");
		}
		else if (foo == 0)
		{
			// Do nothing
		}
		avrPrintf("Stop turning\nOK\n");						//	Tag successful operation to the serial port
		Encoder.MultiActivate = false;							//	Deactivate the flag for thread semaphore
			
	#if 0
		if (foo < 0)
		//if(*p > 0)
		{
			//
			//	Turn Left
			//
			avrPrintf("Trying to turn left\n");
			if(!Dome.turnLeft()) break;
			avrPrintf("Start Turning left\n");
			while(abs(Encoder.Position() - posFinal) > ENCODER_RESOLUTION) nilThdSleepMilliseconds(ROTATE_SLEEP);
			avrPrintf("Stop Turning left\n");
			Dome.stop();
			avrPrintf("Stop OK\n");
		}
		else
		{
			//
			//	Turn Right
			//
			avrPrintf("Trying to turn right\n");
			if(Dome.turnRight()) break;
			avrPrintf("Start Turning Right\n");
			while(abs(Encoder.Position() - posFinal) > ENCODER_RESOLUTION) nilThdSleepMilliseconds(ROTATE_SLEEP);
			avrPrintf("Stop turning right\n");
			Dome.stop();
			avrPrintf("Stop OK\n");
		}
	#endif
		turnfifo.signalFree();									//	Tell FIFO that a slot is free
		//avrPrintf("OK\r\n");									
	}
}
