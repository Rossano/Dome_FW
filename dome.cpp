// 
// 
// 

#include <NilRTOS.h>
#include <NilFIFO.h>

#include "board.h"
#include "dome.h"
#include "encoder.h"
#include "shell.h"

//extern EncoderClass Encoder;

DomeClass::DomeClass()
{
	state = NO_TURN; //DomeState::NO_TURN;
	init();
}

void DomeClass::init()
{
	pinMode(turnLeftPin, OUTPUT);
	digitalWrite(turnLeftPin, LOW);
	pinMode(turnRightPin, OUTPUT);
	digitalWrite(turnRightPin, LOW);
}

DomeStateType DomeClass::getState()
{
	return state;
}

bool DomeClass::turnLeft()
{
	if(state == NO_TURN) 
	{
		state = TURN_LEFT;
		#ifndef ENCODER_SIMULATION
			digitalWrite(turnLeftPin, HIGH);
		#else
			avrPrintf("Start Turning LEFT\n");
			startEncoderTimer();
		#endif
		avrPrintf("Exiting turning left function\n");
		return true;
	}
	return false;
}

bool DomeClass::turnRight()
{
	if(state == NO_TURN)
	{
		state = TURN_RIGHT;
		#ifndef ENCODER_SIMULATION
			digitalWrite(turnRightPin, HIGH);
		#else
			startEncoderTimer();
		#endif
		
		return true;
	}
	return false;
}

void DomeClass::stop()
{
	#ifndef ENCODER_SIMULATION
		if(state == TURN_LEFT) digitalWrite(turnLeftPin, LOW);
		else if (state == TURN_RIGHT) digitalWrite(turnRightPin, LOW);
	#else
		stopEncoderTimer();
	#endif
	
	state = NO_TURN;
}

void TurnLeft(int argc, char *argv[])
{
	(void) argv;
	//        If there are arguments display and error message
	if(argc > 1)
	{
		Usage("turn_left <opt. # of steps>");
		return;
	}
	//        Else  Turning Left
	if(argc == 0)
	{
		Dome.turnLeft();
		avrPrintf("OK\r\n");
	}
	else
	{
		int16_t *p = turnfifo.waitFree(TIME_INFINITE);	
		avrPrintf("argv[0]-> "); avrPrintf(argv[0]);
		//avrPrintf("\nargv[1]-> "); avrPrintf(argv[1]);
		//avrPrintf("\nargv[2]-> "); avrPrintf(argv[2]);
		*p = -atoi(argv[0]);
		avrPrintf("Turning Left of ");
		avrPrintf(*p);
		avrPrintf("\n");
		turnfifo.signalData();
	}
	avrPrintf("OK\r\n");
}

void TurnRight(int argc, char *argv[])
{
	(void) argv;
	//        If there are arguments display and error message
	if(argc > 1)
	{
		Usage("turn_left <opt. # of steps>");
		return;
	}
	//        Else  Turning Left
	if(argc == 0)
	{
		Dome.turnRight();
		avrPrintf("OK\r\n");
	}
	else
	{
		int16_t *p = turnfifo.waitFree(TIME_INFINITE);
		*p = atoi(argv[0]);
		avrPrintf("Turning Right of ");
		avrPrintf(*p);
		avrPrintf("\n");
		turnfifo.signalData();
	}
	avrPrintf("OK\r\n");
}

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
	Dome.stop();
	avrPrintf("OK\r\n");
}

NilFIFO<int16_t, 2> turnfifo;
DomeClass Dome;
extern SEMAPHORE_DECL(EncoderCountSem, 0);

NIL_WORKING_AREA(waDomeThread, STACKSIZE);
NIL_THREAD(DomeThread, arg)
{
	while(TRUE)
	{
		int16_t *p = turnfifo.waitData(TIME_INFINITE);
		
		avrPrintf("Using Dome Thread\nReceived data is: ");
		avrPrintf(*p);
		avrPrintf(CR);
		unsigned long posInitial = Encoder.Position();
		unsigned long posFinal;
		avrPrintf("Dome Initial Position: ");
		avrPrintf(posInitial);
		avrPrintf(CR);
		int16_t foo = *p;
		
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
		
		avrPrintf("Dome Final Position: ");
		avrPrintf(posFinal);
		avrPrintf(CR);
	
		uint16_t count = 0;
		Encoder.MultiActivate = true;
		
		//avrPrintf("foo = ");
		//avrPrintf(foo);
		//avrPrintf(CR);
		if (foo > 0)
		{
			avrPrintf("Start turning Right\n");
			if (!Dome.turnRight())
			{
				avrPrintf("Error: turnRight command\n");
				break;
			}
			while (count <= foo)
			{
				nilSemWait(&EncoderCountSem);
				count++;
				avrPrintf("Count\n");
				// Rearm the semaphore
				nilSemSignal(&EncoderCountSem);
			}
			Dome.stop();
			//avrPrintf("Stop turning Right\n");			
			avrPrintf("Stop turning Right\nOK\n");
		}
		else if (foo < 0)
		{
			avrPrintf("Start turning Left\n");
			if (!Dome.turnLeft())
			{
				avrPrintf("Error: turnLeft command\n");
				break;
			}
			uint8_t end = -foo;
			while (count <= end)//-foo)
			{
				nilSemWait(&EncoderCountSem);
				count++;
				avrPrintf("Count\n");
				//	Rearm the semahaore
				nilSemSignal(&EncoderCountSem);
			}
			Dome.stop();
			//avrPrintf("Error: stop command\n");						
			avrPrintf("Stop Turning Left\nOK\n");
		}
		else if (foo == 0)
		{
			// Do nothing
		}
		avrPrintf("Stop turning\nOK\n");
		Encoder.MultiActivate = false;
			
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
		turnfifo.signalFree();
		avrPrintf("OK\r\n");
	}
}

DomeStateType getDomeState()
{
//	return Dome.state;
}