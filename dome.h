/**
 *  \file dome.h
 *  \brief Arduino Dome header file
 *  This is the implementation of the dome control via an Arduino microcontroller
 */

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
/// \brief Constant Definition
/// \param STACKSIZE size of the thread stack
/// \param ENCODER_RESOLUTION default encoder resolution (not used)
/// \param ROTATE_SLEEP sleep time (in ms) when slewing the dome
///
#define STACKSIZE			64			//	Thread stack size
#define ENCODER_RESOLUTION	1			//	NOT USED
#define ROTATE_SLEEP		10			//	Sleep time while rotating

//////////////////////////////////////////////////////////////////////////
///
///	Type Definition Section
///
//////////////////////////////////////////////////////////////////////////

/// \brief Type defining the state of the Dome Rotation
///
/// \param TURN_LEFT    Dome is turning left
/// \param TURN_RIGHT   Dome is turning right
/// \param NO_TURN      Dome is stopped
///
///
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

///
/// \brief Class to implement and manage the Dome movement
/// 
/// \param DomeStateType *state	Dome rotating state
/// \param shellcmd_t sc_function: pointer to the function implementing the command
///
///
class DomeClass
{
 private:
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

/// \brief Shell Command to turn the Dome left.
/// Activate the motor to turn anticlockwise the Dome.
///
/// \param argc int Arguments number
/// \param argv[] char* Pointer to the list of arguments
/// \return void
///
void TurnLeft(int argc, char *argv[]);					//        Shell command to Turn Dome to the left
/// \brief Shell Command to turn the Dome right.
/// Activate the motor to turn clockwise the Dome
///
/// \param argc int Arguments number
/// \param argv[] char* Pointer to list of arguments
/// \return void
///
void TurnRight(int argc, char *argv[]);					//        Shell command to Turn Dome to the right
/// \brief Shell Command to stop turning the Dome.
/// Deactivate the motor that is actually turning the Dome
///
/// \param argc int Argument number
/// \param argv[] char* Pointer to the list of arguments
/// \return void
///
void Stop(int argc, char *argv[]);						//        Shell command to Stop turning the Dome
/**
 *  \brief Shell Command to read the Dome turning state.
 *  This command checks the passed arguments and returns on terminal the
 *  actual encoder absolute position.
 *  \param [in] argc int Number of arguments
 *  \param [in] argv char[]* Arguments list
 *  \return void
 *  
 *  \details This function ask the state to the dome object and returns it on serial port
 */
void getState(int argc, char *argv[]);
/**
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
/**
 *  \brief Wrap up  command to return the Dome state.
 *  Dummy wrap up to access and simplify access to the Dome state information.
 *  \return DomeStateType Dome Turning State
 *  
 *  \details This function returns the state from the Dome object
 */
DomeStateType getDomeState();						//	Wrap up function to get the Dome rotating state

#ifndef ENCODER_SIMULATION
	DomeStateType getDomeState();						//	Wrap up function to get the Dome rotating state
#endif

#endif

