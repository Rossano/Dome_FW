/**
 *  \file dome.h
 *  \author Rossano Pantaleoni
 *  \version 0.6.1.3
 *  \date 11 Apr 2014
 *  \brief Arduino Dome header file
 *
 *  This file implements the interface of an Arduino control of the telescope Dome
 */

#ifndef _DOME_h
#define _DOME_h

//////////////////////////////////////////////////////////////////////////
//
//	Class Definition Section
//
//////////////////////////////////////////////////////////////////////////

//
//	Arduino Module Inclusion
//
#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <NilFIFO.h>					//	FIFO module of RTOS inclusion

//////////////////////////////////////////////////////////////////////////
//
//	Definition Section
//
//////////////////////////////////////////////////////////////////////////

/** \brief Constant Definition
 *
 * \param STACKSIZE size of the thread stack
 * \param ENCODER_RESOLUTION default encoder resolution (not used)
 * \param ROTATE_SLEEP sleep time (in ms) when slewing the dome
 *
 */
#define STACKSIZE			32//64			//	Thread stack size
#define ENCODER_RESOLUTION	1			//	NOT USED
#define ROTATE_SLEEP		10			//	Sleep time while rotating

//////////////////////////////////////////////////////////////////////////
//
//	Type Definition Section
//
//////////////////////////////////////////////////////////////////////////

/** \enum DomeStateType
 *  \brief Type defining the state of the Dome Rotation
 *
 *  \param TURN_LEFT    Dome is turning left
 *  \param TURN_RIGHT   Dome is turning right
 *  \param NO_TURN      Dome is stopped
 *
 *  \detail This type enumerates the Dome turning state conditions, more in detail
 *  if the dome is turning left or right, or if it is not turning at all.
 */
typedef enum
{
	TURN_LEFT,							//	Dome is turning on LEFT (Encoder decreasing)
	TURN_RIGHT,							//	Dome is turning on RIGHT (Encoder increasing)
	NO_TURN								//	Dome is not turning
} DomeStateType;


//////////////////////////////////////////////////////////////////////////
//
//	Class Definition Section
//
//////////////////////////////////////////////////////////////////////////

/** \class DomeClass
 *  \brief Class to implement and manage the Dome movement
 *
 *  \detail This class implements the code interface for the telescope dome, in particular
 *  it takes care to:
 *  - Store the Dome turning state;
 *  - Act as actuator to turn the dome left right;
 *  - Stop the Dome from turning
 */
class DomeClass
{
 private:

    /** \brief [PRIVATE] DomeStateType *state	Dome rotating state
     *
     */
	DomeStateType state;				//	State of the Dome

 public:
	 /**
	 *  \brief Default Constructor
	 *  \details It initialize the DomeCLass data structure
	 */
	DomeClass();						//	Default class constructor
	/**
	 *  \brief Dome HW configuration. It sets up the Arduino I/O for the motor control
	 *
	 *  \return void
	 *
	 *  \details It initialize the Arduino HW for the Dome application
	 */
	void init();						//	Method to initialize the Dome class
	/**
	 *  \brief Turn the Dome left. This method drive the motor to turn anticlockwise
	 *
	 *  \return bool TRUE if operation successful, FALSE otherwise
	 *
	 *  \details This method activate the anticlockwise dome rotation
	 */
	bool turnLeft();					//	Method to activate the motor to turn the Dome on the left
	/**
	 *  \brief Turn the Dome right. This method drive the motor to turn clockwise
	 *
	 *  \return bool TRUE if operation successful, FALSE otherwise
	 *
	 *  \details This method activates the clockwise dome rotation
	 */
	bool turnRight();					//	Method to activate the motor to turn the Dome on the right
	/**
	 *  \brief Method to stop turning the Dome
	 *
	 *  \return void
	 *
	 *  \details This method stops the rotation of the dome
	 */
	void stop();						//	Method to deactivate the motor
	/**
	 *  \brief Returns the Dome state
	 *
	 *  \return DomeStateType Dome rotating state
	 *
	 *  \details Method to return the Dome rotating state, simple way to expose the state private
	 *  member
	 */
	DomeStateType getState();			//	Method to return the Dome turning state
};

//////////////////////////////////////////////////////////////////////////
//
//	Variable Section
//
//////////////////////////////////////////////////////////////////////////

/** \def NilFIFO<int16_t, 2> turnfifo
 *  \brief FIFO use to pass arguments (the number of steps to turn) to the dome thread.
*   This FIFO is used to synchronize the Dome thread when slewing a fixed number of steps
 *  To note that this feature it is NOT IMPLEMENTED!
 *
 */

extern NilFIFO<int16_t, 2> turnfifo;					//	FIFO used to pass the # of turning steps
/** \def DomeClass Dome
 *  \brief Dome class object
 *
 */
extern DomeClass Dome;									//	Dome data structure

//////////////////////////////////////////////////////////////////////////
//
//	Prototype Section
//
//////////////////////////////////////////////////////////////////////////

/** \fn void TurnLeft(int argc, char *argv[])
 *  \brief Shell Command to turn the Dome left.
 *
 *  \param argc int Arguments number
 *  \param argv[] char* Pointer to the list of arguments
 *  \return void
 *
 *  \details Activate the motor to turn anticlockwise the Dome.
 */
void TurnLeft(int argc, char *argv[]);					// Shell command to Turn Dome to the left

/** \fn void TurnRight(int argc, char *argv[])
 *  \brief Shell Command to turn the Dome right.
 *
 *  \param argc int Arguments number
 *  \param argv[] char* Pointer to list of arguments
 *  \return void
 *
 *  \details Activate the motor to turn clockwise the Dome
 */
void TurnRight(int argc, char *argv[]);					// Shell command to Turn Dome to the right

/** \fn void Stop(int argc, char *argv[])
 *  \brief Shell Command to stop turning the Dome.
 *
 *  \param argc int Argument number
 *  \param argv[] char* Pointer to the list of arguments
 *  \return void
 *
 *  \details Deactivate the motor that is actually turning the Dome
 *
 */
void Stop(int argc, char *argv[]);						// Shell command to Stop turning the Dome

/** \fn void getState(int argc, char *argv[])
 *  \brief Shell Command to read the Dome turning state.
 *  \param [in] argc int Number of arguments
 *  \param [in] argv char[]* Arguments list
 *  \return void
 *
 *  \details This function ask the state to the dome object and returns it on serial port
 *  This command checks the passed arguments and returns on terminal the
 *  actual encoder absolute position.
 */
void getState(int argc, char *argv[]);

/** \fn void gearCfg(int argc, char *argv[])
 *  \brief Shell Command to configure the Dome mechanical gear.
 *  This command checks the passed arguments and returns on terminal the
 *  actual encoder absolute position
 *  \param [in] argc int Number of command arguments
 *  \param [in] argv char[]* list of arguments
 *  \return void
 *
 *  \details This function configures the encoder object to the value of the dome mechanical system.
 */
void gearCfg(int argc, char *argv[]);

/*
 *  \brief Shell Command to configure the Dome in debug mode.
 *  Debug mode is when the encoder HW is not physically present and it is then
 *  simulated. No parameter returns the actual status, else it looks for ON/OFF to set clear the debug mode
 *  \param [in] argc int Number of command arguments
 *  \param [in] argv char[]* list of arguments
 *  \return void
 *
 *  \details This function configures the encoder object to the value of the dome mechanical system.
 */
//void debugMode(int argc, char *argv[]);

/** \fn DomeStateType getDomeState()
 *  \brief Wrap up  command to return the Dome state.
 *  Dummy wrap up to access and simplify access to the Dome state information.
 *  \return DomeStateType Dome Turning State
 *
 *  \details This function returns the state from the Dome object
 */
DomeStateType getDomeState();						    //	Wrap up function to get the Dome rotating state

#endif

