// dome.h

#ifndef _DOME_h
#define _DOME_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <NilFIFO.h>

#define STACKSIZE			64
#define ENCODER_RESOLUTION	2
#define ROTATE_SLEEP		10

//const uint8_t turnLeftPin = 8;
//const uint8_t turnRightPin = 9;

typedef enum/*  DomeState*/
{
	TURN_LEFT,
	TURN_RIGHT,
	NO_TURN
} DomeStateType;

//DomeStateType DomeState;

class DomeClass
{
 private:
	DomeStateType state;

 public:
	DomeClass();
	void init();
	bool turnLeft();
	bool turnRight();
	void stop();
	DomeStateType getState();
};

extern NilFIFO<int16_t, 2> turnfifo;
extern DomeClass Dome;

void TurnLeft(int argc, char *argv[]);					//        Turn Dome to the left
void TurnRight(int argc, char *argv[]);					//        Turn Dome to the right
void Stop(int argc, char *argv[]);						//        Stop turning the Dome

#ifndef ENCODER_SIMULATION
	DomeStateType getDomeState();
#endif

#endif

