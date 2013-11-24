//////////////////////////////////////////////////////////////////////////
///
///		encoder.h
///
//////////////////////////////////////////////////////////////////////////

#ifndef _ENCODER_h
#define _ENCODER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

//////////////////////////////////////////////////////////////////////////
///
///	Defines Section
///
//////////////////////////////////////////////////////////////////////////

#define STACKSIZE	64				//	Thread stacksize
///
///		Default definitions for the Encoder Configuration
/// 
#define ENCODER_RESOLUTION	16		//	Resolution of the Encoder
#define ENCODER_GEAR_RATIO  1.0		//	Ratio between Encoder Gear and Final Gear
//	Max counts to make a full turn of the final gear
#define ENCODER_MAX_COUNT	floor(ENCODER_RESOLUTION * ENCODER_GEAR_RATIO)
//	Encoder interface: A_ONLY, A_AND_B,  FULL
#define ENCODER_IMPLEMENTATION	A_ONLY
#define MAX_COUNT	9				//	Max counts to make a full turn of the final gear
#define ENCODER_SIMULATION			//	Activate the simulation of the encoder
#define RETURN_ANGLE		1		//	0-> Activate the position return as circular buffer count
									//	1-> Activate the position return as angle

//////////////////////////////////////////////////////////////////////////
///
///	Class Definition Section
///
//////////////////////////////////////////////////////////////////////////

/// <summary>
/// Class to implement and manage the Incremental Encoder.
/// </summary>
/// <param name="encoderResolution">Stores the encoder pulses per turn</param>
/// <param name="gearRatio">Stores the encoder transmission ratio.</param>
/// <param name="encoderMaxCount">Stores the encoder circular buffer lenght.</param>
/// <param name="MultiActivate">Stores activation flag for the automatic step turning.</param>
/// <param name="_positiono">[Private] Stores the encoder absolute position.</param>
/// <param name="EncoderClass()">Default Constructor.</param>
/// <param name="EncoderClass(pos)">Constructor setting the position.</param>
/// <param name="init()">Method to initialize the Encoder data structure.</param>
/// <param name="Position()">Method to read the encoder absolute position.</param>
/// <param name="SetPosition()">Method to set the Encoder absolute position.</param>
/// <param name="channelA()">Method to return the Encoder A channel state.</param>
/// <param name="channelB()">Method to return the Encoder B channel state.</param>
/// <param name="channelHome()">Method to return the HOME channel state.</param>
/// <param name="operator ++">Method to Increase the Encoder absolute position.</param>
/// <param name="operator --">Method to Decrease the Encoder absolute position.</param>
class EncoderClass
{
 public:
	uint16_t encoderResolution;				//	Resolution of the Encoder
	double gearRatio;						//	Encoder Gear / Final Gear ration => must be > 1
	uint32_t encoderMaxCount;				//	# of pulses to make a full turn of the final gear
 private:
	uint32_t _position;						//	Actual absolute Encoder position
	
 public:
	EncoderClass();							//	Default Constructor
	EncoderClass(uint32_t pos);				//	Constructor setting up the Encoder absolute position
	void init();							//	Initialize the encoder HW and data structure
	uint32_t Position();					//	Returns the actual Encoder absolute position
	void SetPosition(unsigned long pos);	//	Set the encoder position
	bool channelA();						//	Returns encoder signal A state
	bool channelB();						//	Returns encoder signal B state
	bool channelHome();						//	Returns HOME flag state
	//	Operators ++ & -- implementation to have more readable code 
	EncoderClass& operator++();
	EncoderClass operator++(int);
	EncoderClass& operator--();
	EncoderClass operator--(int);
	bool MultiActivate;						//	Used to tells code that a fixed # of turns have to be done
};

extern EncoderClass Encoder;

//////////////////////////////////////////////////////////////////////////
///
///	Function Prototype Section
///
//////////////////////////////////////////////////////////////////////////

void encoderISR();							//	Encoder pulse reception ISR
void homeISR();								//	Home pulse reception ISR
void getPosition(int argc, char *argv[]);	//	Shell command to read the encoder position

//////////////////////////////////////////////////////////////////////////
///	Function Wraps-up to start/stop the encoder timer 
///	only used when simulating the Encoder
///	(to avoid to export too many data structures)
//////////////////////////////////////////////////////////////////////////
#ifdef ENCODER_SIMULATION
	void startEncoderTimer();				//	Start the encoder simulation timer
	void stopEncoderTimer();				//	Stop the encoder simulation timer
#endif // ENCODER_SIMULATION

#endif

