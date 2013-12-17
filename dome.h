// dome.h

#ifndef _DOME_h
#define _DOME_h

//////////////////////////////////////////////////////////////////////////
///
///	Class Definition Section
///
//////////////////////////////////////////////////////////////////////////

///
///	Arduino Module Inclusion	
///
#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <NilFIFO.h>					//	FIFO module of RTOS inclusion

//////////////////////////////////////////////////////////////////////////
///
///	Definition Section
///
//////////////////////////////////////////////////////////////////////////

#define STACKSIZE			64			//	Thread stack size
#define ENCODER_RESOLUTION	1			//	NOT USED
#define ROTATE_SLEEP		10			//	Sleep time while rotating

//////////////////////////////////////////////////////////////////////////
///
///	Type Definition Section
///
//////////////////////////////////////////////////////////////////////////

///<summary>
/// Type defining the state of the Dome Rotation
///</summary>
typedef enum
{
	TURN_LEFT,							//	Dome is turning on LEFT (Encoder decreasing)
	TURN_RIGHT,							//	Dome is turning on RIGHT (Encoder increasing)
	NO_TURN								//	Dome is not turning
} DomeStateType;


//////////////////////////////////////////////////////////////////////////
///
///	Class Definition Section
///
//////////////////////////////////////////////////////////////////////////

/// <summary>
/// Class to implement and manage the Dome movement.
/// </summary>
/// <param name="state">[PRIVATE]; Dome Rotation state</param>
/// <param name="DomeClass()">Default Constructor.</param>
/// <param name="init()">Method to initialize the Dome data structure.</param>
/// <param name="turnLeft()">Method to turn the Dome on the left (anti-clockwise).</param>
/// <param name="turnRight()">Method to turn the Dome on the right (clockwise).</param>
/// <param name="stop()">Method to stop turning the Dome.</param>
/// <param name="getState()">Method to return the Dome turning state.</param>
class DomeClass
{
 private:
	DomeStateType state;				//	State of the Dome

 public:
	DomeClass();						//	Default class constructor
	void init();						//	Method to initialize the Dome class
	bool turnLeft();					//	Method to activate the motor to turn the Dome on the left
	bool turnRight();					//	Method to activate the motor to turn the Dome on the right
	void stop();						//	Method to deactivate the motor 
	DomeStateType getState();			//	Method to return the Dome turning state
};

//////////////////////////////////////////////////////////////////////////
///
///	Variable Section
///
//////////////////////////////////////////////////////////////////////////

extern NilFIFO<int16_t, 2> turnfifo;					//	FIFO used to pass the # of turning steps
extern DomeClass Dome;									//	Dome data structure

//////////////////////////////////////////////////////////////////////////
///
///	Prototype Section
///
//////////////////////////////////////////////////////////////////////////

void TurnLeft(int argc, char *argv[]);					//        Shell command to Turn Dome to the left
void TurnRight(int argc, char *argv[]);					//        Shell command to Turn Dome to the right
void Stop(int argc, char *argv[]);						//        Shell command to Stop turning the Dome
void getState(int argc, char *argv[]);
void gearCfg(int argc, char *argv[]);

DomeStateType getDomeState();						//	Wrap up function to get the Dome rotating state

#ifndef ENCODER_SIMULATION
	DomeStateType getDomeState();						//	Wrap up function to get the Dome rotating state
#endif

#endif

