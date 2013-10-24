// 
// 
// 

#include <NilRTOS.h>
#include <NilFIFO.h>

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
	if(state == NO_TURN) //DomeState::NO_TURN)
	{
		state = TURN_LEFT;//DomeState::TURN_LEFT;
		digitalWrite(turnLeftPin, HIGH);
		return true;
	}
	return false;
}

bool DomeClass::turnRight()
{
	if(state == NO_TURN)//DomeState::TURN_RIGHT)
	{
		state = TURN_RIGHT;//DomeState::TURN_RIGHT;
		digitalWrite(turnRightPin, HIGH);
		return true;
	}
	return false;
}

void DomeClass::stop()
{
	//if(state == DomeState::TURN_LEFT) digitalWrite(turnLeftPin, LOW);
	//else if (state == DomeState::TURN_RIGHT) digitalWrite(turnRightPin, LOW);
	if(state == TURN_LEFT) digitalWrite(turnLeftPin, LOW);
	else if (state == TURN_RIGHT) digitalWrite(turnRightPin, LOW);
	state = NO_TURN;
}

void TurnLeft(int argc, char *argv[])
{
	(void) argv;
	//        If there are arguments display and error message
	if(argc > 0)
	{
		Usage("turn_left <opt. # of steps>");
		return;
	}
	//        Else  Turning Left
	if(argc == 1)
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
	if(argc > 0)
	{
		Usage("turn_left <opt. # of steps>");
		return;
	}
	//        Else  Turning Left
	if(argc == 1)
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
		
		unsigned long posInitial = Encoder.Position();
		unsigned long posFinal;
		
		if(*p > 0)
		{
			//
			//	Turn Left : carry out final position to reach
			//
			if(posInitial + *p > MAX_COUNT) posFinal = posInitial + *p - MAX_COUNT;
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