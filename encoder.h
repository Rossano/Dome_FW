/**
 *  \file encoder.h
 *  \author Rossano Pantaleoni
 *  \version 0.6.1.2
 *  \date 9 Apr 2014
 *  \brief Arduino Encoder header file
 *
 *  This file implements the interface of an Arduino control of an incremental encoder
 */
 //////////////////////////////////////////////////////////////////////////
//
//		encoder.h
//
//////////////////////////////////////////////////////////////////////////

#ifndef _ENCODER_h
#define _ENCODER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

//////////////////////////////////////////////////////////////////////////
//
//	Defines Section
//
//////////////////////////////////////////////////////////////////////////

/** \brief Constant Definition
 *
 * \def STACKSIZE size of the thread stack
 * \def ENCODER_RESOLUTION default encoder resolution
 * \def ENCODER_GEAR_RATIO gear ratio of the the dome/encoder mechanical system
 * \def ENCODER_MAX_COUNT Number of encoder pulses to make a full turn of the dome
 * (floor(ENCODER_RESOLUTION * ENCODER_GEAR_RATIO))
 * \def ENCODER_IMPLEMENTATION Incremental encoder coded interface, possible choices:
 * A_ONLY: only a single pulse is managed
 * A_AND_B: 2 quadrature signals are managed
 * FULL: 2 quadrature plus Z signals are managed
 * \def ENCODER_SIMULATION compile flag to activate the software simulation of the encoder (debug)
 * \def RETURN_ANGLE compile flag to set to provide the return position as angle (1) or as counter (0)
 */
#define STACKSIZE	32//64				    //	Thread stacksize
//
//		Default definitions for the Encoder Configuration
//
#define ENCODER_RESOLUTION	100//16000	        //	Resolution of the Encoder
#define ENCODER_GEAR_RATIO  80//1.0		        //	Ratio between Encoder Gear and Final Gear
///	Max counts to make a full turn of the final gear
#define ENCODER_MAX_COUNT	floor(ENCODER_RESOLUTION * ENCODER_GEAR_RATIO)
//	Encoder interface: A_ONLY, A_AND_B,  FULL
#define ENCODER_IMPLEMENTATION	A_ONLY
#define MAX_COUNT	8000				//	Max counts to make a full turn of the final gear
//#define ENCODER_SIMULATION			//	Activate the simulation of the encoder
#define RETURN_ANGLE		0		//	0-> Activate the position return as circular buffer count
                                                //	1-> Activate the position return as angle

typedef enum
{
	POSITIVE,
	NEGATIVE	
} encoderPolarity_t;

//////////////////////////////////////////////////////////////////////////
//
//	Class Definition Section
//
//////////////////////////////////////////////////////////////////////////

/**\class EncoderClass
 *  \brief Class to implement and manage the Incremental Encoder.
 *
 *  This class encloses all the operations to manage an incremental encoder:
 *  - It sets-up the encoder configuration (resolution, gear ration and so on and so worht);
 *  - It retrieve the position from external interrupts;
 *  - It provide the position when required;
 */
class EncoderClass
{
 public:
    /** \brief Stores the encoder pulses per turn
     *
     */
	uint16_t encoderResolution;				//	Resolution of the Encoder
    /** \brief Stores the encoder transmission ratio.
     *
     */
	double gearRatio;						//	Encoder Gear / Final Gear ration => must be > 1
    /** \brief Stores the encoder circular buffer lenght.
     *
     */
	uint32_t encoderMaxCount;				//	# of pulses to make a full turn of the final gear
    /** \brief Encoder detection polarity
     *
     *  \details This variable allow the encoder to count up and down seamlessy from
     *  the actual hardware connection. For instance, if after wiring the system it is
     *  found that the encoder and slewing are inverted, setting up this fields
     *  can fix the problem without the need of re-wiring.
     */
//	uint8_t polarity;				//	# Encoder counts polarity detection (and direction)
    /** \brief Stores activation flag for the automatic step turning.
     *
     */
	bool MultiActivate;	                    //	Used to tells code that a fixed # of turns have to be done
 private:
    /** \brief Stores the encoder absolute position.
     *
     */
	uint32_t _position;						//	Actual absolute Encoder position

	encoderPolarity_t polarity;
	
 public:
	/** \brief Default Constructor
     */
	EncoderClass();							//	Default Constructor
	/** \brief Constructor Initializing the Encoder position.
     *
     *  \param pos uint32_t Initial position of the encoder
     *
     */
	EncoderClass(uint32_t pos);				//	Constructor setting up the Encoder absolute position
	/** \brief Initialize the Encoder HW and EncoderClass data structure.
     *
     *  \return void
     *
     */
	void init();							//	Initialize the encoder HW and data structure
	/** \brief Method to read the Encoder position
     *
     *  \return uint32_t Encoder actual absolute position
     *
     */
	uint32_t Position();					//	Returns the actual Encoder absolute position
	/** \brief Method to Set the Encoder position
     *
     *  \param pos uint32_t New position of the encoder
     *  \return void
     *
     */
	void SetPosition(unsigned long pos);	//	Set the encoder position
	/** \brief Method to read the Encoder A signal state
     *
     *  \return bool Encoder A channel state
     *
     */
	bool channelA();						//	Returns encoder signal A state
    /** \brief Method to read the Encoder B signal state
     *
     *  \return bool Encoder B channel state
     *
     */
    bool channelB();						//	Returns encoder signal B state
    /** \brief Method to read the HOME signal state
     *
     *  \return bool HOME signal state
     *
     */
	bool channelHome();						//	Returns HOME flag state
	//	Operators ++ & -- implementation to have more readable code

	/** \brief Operator ++
     *
     *  \return EncoderClass& EncoderClass::operator New EncoderClass data structure
     *
     */
	EncoderClass& operator++();
	/** \brief Operator ++
     *
     *  \param int
     *  \return EncoderClass EncoderClass::operator EncoderClass incremented
     *
     */
	EncoderClass operator++(int);
	/** \brief Operator --
     *
     *  \return EncoderClass& EncoderClass::operator New EncoderClass data structure
     *
     */
	EncoderClass& operator--();
	/** \brief Operator --
     *
     *  \param int
     *  \return EncoderClass EncoderClass::operator EncoderClass decremented
     *
     */
	EncoderClass operator--(int);
	
	void setPolarity(encoderPolarity_t pol);
	encoderPolarity_t getPolarity();
};

extern EncoderClass Encoder;

extern bool encoderDebugMode;
extern bool use_encoder_sem;
extern bool isConnected;
//////////////////////////////////////////////////////////////////////////
//
//	Function Prototype Section
//
//////////////////////////////////////////////////////////////////////////

/** \fn encoderISR()
 *  \brief External Interrupt Service Routine for Encoder
 *
 *  \details The code is triggered when there is an external event on the Encoder input ports
 *  It checks the status of the encoder port A, B and Home to increment the encoder
 *  absolute position.
 *  This ISR is run when there is a pulse activity on and external pad
 *  due to encoder activity.
 *  It updates the position counter and then exits.
 *
 * \return void
 *
 */
void encoderISR();							//	Encoder pulse reception ISR

/** \fn homeISR()
 *  \brief External Interrupt Service Routine for Home position
 *
 *  \details The code is triggered when there is an external event on the Home input ports
 *  It resets the increment the encoder position to 0
 *  This ISR is run when there is a pulse activity on and external pad
 *  due to home pulse detection.
 *  It sets position counter to zero and then exits.
 *
 * \return void
 *
 */
void homeISR();								//	Home pulse reception ISR

/** \fn getPosition(int argc, char *argv[])
 *  \brief Shell Command to read the Encoder position.
 *
 *  \param argc int Number of arguments
 *  \param argv[] char* List of arguments
 *  \return void
 *
 *  \details This command checks the passed arguments and returns on terminal the
 *  actual encoder absolute position.
 */
void getPosition(int argc, char *argv[]);	//	Shell command to read the encoder position

/** \fn setPosition(int argc, char *argv[])
 *  \brief Shell command to set the current dome position
 *
 *  \param argc int Number of arguments
 *  \param argv[] char* List of arguments
 *  \return void
 *
 */
void setPosition(int argc, char *argv[]);   // Shell command to set the encoder position
void getState(int argc, char *argv[]);      // Shell command to get the actual slewing state
void gearCfg(int argc, char *argv[]);       // Shell command to configure the encoder

/**\fn  setEncoderPalarity(int argc, char *argv[])
 *  \brief Shell Command to configure the Encoder Detection Polarity.
 *  This shell command is used to configure the Encoder polarity field.
 *  \param [in] argc int Number of command arguments
 *  \param [in] argv char[]* list of arguments
 *  \return void
 *
 *  \details Please refer to EncoderClass polarity filed for more details.
 */
void setEncoderPolarity(int argc, char *argv[]);

/**\fn debugMode(int argc, char *argv[])
 *  \brief Shell Command to configure the Dome in debug mode.
 *  Debug mode is when the encoder HW is not physically present and it is then
 *  simulated. No parameter returns the actual status, else it looks for ON/OFF to set clear the debug mode
 *  \param [in] argc int Number of command arguments
 *  \param [in] argv char[]* list of arguments
 *  \return void
 *
 *  \details This function configures the encoder object to the value of the dome mechanical system.
 */
void debugMode(int argc, char *argv[]);

//////////////////////////////////////////////////////////////////////////
//	Function Wraps-up to start/stop the encoder timer
//	only used when simulating the Encoder
//	(to avoid to export too many data structures)
//////////////////////////////////////////////////////////////////////////
/**\fn startEncoderTimer()
 *  \brief Function Wraps-up to start the encoder timer
 *
 *  \details Function Wrap up to easily start the NilRTOS timer.
 *
 *  \return void
 *
 */
void startEncoderTimer();				    //	Start the encoder simulation timer

/** \fn stopEncoderTimer()
 *  \brief Function Wraps-up to stop the encoder timer
 *
 *  \return void
 *
 *  \details Function wrap-up to easily stop the NilRTOS timer
 */
void stopEncoderTimer();				    //	Stop the encoder simulation timer

#endif

