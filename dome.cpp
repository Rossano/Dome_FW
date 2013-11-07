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
		*p = atoi(argv[2]);
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
		*p = atoi(argv[2]);
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

NIL_WORKING_AREA(waDomeThread, STACKSIZE);
NIL_THREAD(DomeThread, arg)
{
	while(TRUE)
	{
		int16_t *p = turnfifo.waitData(TIME_INFINITE);
		
		avrPrintf("Using Dome Thread\n");
		unsigned long posInitial = Encoder.Position();
		unsigned long posFinal;
		
		if(*p > 0)
		{
			//
			//	Turn Left : carry out final position to reach
			//
			if(posInitial + *p > MAX_COUNT-1) posFinal = posInitial + *p - MAX_COUNT;
			else posFinal = posInitial + *p;
		}
		else
		{
			//
			//	Turn Right : carry out final position to reach
			//
			if(posInitial < *p) posFinal = MAX_COUNT + posInitial + *p;
			else posFinal = posInitial + *p;
		}
		
		if(*p > 0)
		{
			//
			//	Turn Left
			//
			if(Dome.turnLeft()) break;
			while(abs(Encoder.Position() - posFinal) < ENCODER_RESOLUTION) nilThdSleepMilliseconds(ROTATE_SLEEP);
			Dome.stop();
		}
		else
		{
			//
			//	Turn Right
			//
			if(Dome.turnRight()) break;
			while(abs(Encoder.Position() - posFinal) < ENCODER_RESOLUTION) nilThdSleepMilliseconds(ROTATE_SLEEP);
			Dome.stop();
		}
		turnfifo.signalFree();
		avrPrintf("OK\r\n");
	}
}

DomeStateType getDomeState()
{
//	return Dome.state;
}