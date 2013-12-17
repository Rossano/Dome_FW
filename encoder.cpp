/**
 *  \file encoder.h
 *  \brief Arduino Encoder code
 *  This file implements the interface of an Arduino control of an incremental encoder
 */

//////////////////////////////////////////////////////////////////////////
///
///	Include Section
///
//////////////////////////////////////////////////////////////////////////

#include <NilRTOS.h>		//	RTOS inclusion

#include "board.h"			//	Board definition inclusion
#include "encoder.h"		//	Encoder Code inclusion
#include "shell.h"			//	Shell Code inclusion

#undef MEMORY_CHECK
#ifdef MEMORY_CHECK
	#include "MemoryFree.h"	//	Memory Check Global Code inclusion
#endif

//////////////////////////////////////////////////////////////////////////
///
///	Defines Section
///
//////////////////////////////////////////////////////////////////////////

#undef DEBUG								//	DEBUG FLAG
#define ENCODER_SIMULATION					//	FLAG to simulate the Encoder 
#define DEBUG_TIMER_INTERVAL_US		3000000	//	Simulate Encoder time
#undef TIMER_DEBUG							//	FLAG to activate the Simulated Encoder debug

//
//	If Encoder simulation is used it is mandatory to include the 
//	RTOS timer module
//
#ifdef ENCODER_SIMULATION 
	#include <NilTimer1.h>					//	NilRTOS Timer module inclusion
#endif // ENCODER_SIMULATION

//////////////////////////////////////////////////////////////////////////
///
///	Variable Section
///
//////////////////////////////////////////////////////////////////////////

/// \brief Encoder class object
/// This variable is the entry point for all the incremental encoder management
///
EncoderClass Encoder;						//	Encoder data structure

//
//	If Encoder simulation define a semaphore to synchronize the Encoder thread
//	and include the Dome data structure for the turning direction
//	Else just declare a semaphore to synchronize Encoder thread
//
#ifdef ENCODER_SIMULATION 
	#include "dome.h"						//	Include the Dome module
	SEMAPHORE_DECL(DebugSem, 0);			//	Semaphore indicating Encoder thread that a new pulse arrived
	extern DomeClass Dome;
#else
	SEMAPHORE_DECL(encoderSem, 0);			//	Semaphore indicating Encoder thread of a new pulse
#endif // ENCODER_SIMULATION

/// \brief Semaphore used to synchronized the counting of slewing steps
///
/// \param EncoderCountSem semaphore name
/// \param 0 semaphore initial count (free)
///
static SEMAPHORE_DECL(EncoderCountSem, 0);	//	Semaphore used when counting fixed # of steps

//////////////////////////////////////////////////////////////////////////
///
///	Code Section
///
//////////////////////////////////////////////////////////////////////////

extern void avrPrintf(const char * str);	//	function to print to terminal a string

/// \brief Default Constructor
///
EncoderClass::EncoderClass()
{
	_position = 0;
	init();
}

/// \brief Constructor Initializing the Encoder position.
///
/// \param pos uint32_t Initial position of the encoder
///
EncoderClass::EncoderClass(uint32_t pos)
{
	_position = pos;
	init();		
}

/// \brief Initialize the Encoder HW and EncoderClass data structure.
///
/// \return void
///
void EncoderClass::init()
{	
	MultiActivate = false;							//	No fix # of step by default
	encoderResolution = ENCODER_RESOLUTION;			//	Init Encoder resolution
	gearRatio = ENCODER_GEAR_RATIO;					//	Init system gear ratio
	double foo = encoderResolution * gearRatio;		
	encoderMaxCount = 15;//(uint32_t)foo;
	#ifdef DEBUG
		avrPrintf("Encoder MAX cnt -> ");
		avrPrintf(encoderMaxCount);
		//	avrPrintf(foo);
		avrPrintf(CR);
	#endif // DEBUG
	//
	//	Initialize the Arduino pins for Encoder HW
	//
	pinMode(encoderA, INPUT_PULLUP);
	pinMode(encoderB, INPUT_PULLUP);
	pinMode(encoderHome, INPUT_PULLUP);
	digitalWrite(encoderA, HIGH);
	digitalWrite(encoderB, HIGH);
	digitalWrite(encoderHome, HIGH);
	//
	//	If Encoder is not simulated attach IRS to the Encoder external pin IRQ
	//
	#ifdef ENCODER_SIMULATION 
		///	DO NOTHING		
	#else	
		///	Configure IRQ on external pins
		attachInterrupt(IRQ_PINA, encoderISR, RISING);
		#if (ENCODER_IMPLEMENTATION = A_AND_B)
		//	Configure interrupt on B port only if it is used
			attachInterrupt(IRQ_PINB, encoderISR, RISING);
		#endif
		attachInterrupt(IRQ_HOME, homeISR, RISING);
	#endif //ENCODER_SIMULATION		
}

/// \brief Method to read the Encoder position
///
/// \return uint32_t Encoder actual absolute position
///
///
uint32_t EncoderClass::Position()
{
	return _position;
}

/// \brief Method to Set the Encoder position
///
/// \param pos uint32_t New position of the encoder
/// \return void
///
void EncoderClass::SetPosition(uint32_t pos)
{
	//	Since it is a critical parameter better to updated under a lock block
	nilSysLock();
	_position = pos;
	nilSysUnlock();
}

/// \brief Method to read the Encoder A signal state
///
/// \return bool Encoder A channel state
///
bool EncoderClass::channelA()
{
	return digitalRead(encoderA);
}

/// \brief Method to read the Encoder B signal state
///
/// \return bool Encoder B channel state
///
bool EncoderClass::channelB()
{
	return digitalRead(encoderB);
}

/// \brief Method to read the HOME signal state
///
/// \return bool HOME signal state
///
bool EncoderClass::channelHome()
{
	return digitalRead(encoderHome);
}

//
//	Definition of operators ++ & -- to change the Encoder position in a more elegant way
//

/// \brief Operator ++
///
/// \return EncoderClass& EncoderClass::operator New EncoderClass data structure
///
EncoderClass& EncoderClass::operator ++()
{
	return *this;
}

/// \brief Operator ++
///
/// \param int
/// \return EncoderClass EncoderClass::operator EncoderClass incremented
///
EncoderClass EncoderClass::operator ++(int)
{
	if(_position == encoderMaxCount)
	{
		//	If max position is reached loop back to 0
		_position = 0;
		return 0;
	}
	else
	{
		//	Else simply increase it
		_position++;
		return _position;
	}
}

/// \brief Operator --
///
/// \return EncoderClass& EncoderClass::operator New EncoderClass data structure
///
EncoderClass& EncoderClass::operator --()
{
	return *this;
}

/// \brief Operator --
///
/// \param int
/// \return EncoderClass EncoderClass::operator EncoderClass decremented
///
EncoderClass EncoderClass::operator --(int)
{
	if(_position == 0)
	{
		//	If 0 is reached loop back to the MAX value
		_position = encoderMaxCount;
		return encoderMaxCount;
	}
	else
	{
		//	else simply decrease
		_position--;
		return _position;
	}
}

/// \brief Shell Command to read the Encoder position.
/// This command checks the passed arguments and returns on terminal the
/// actual encoder absolute position.
///
/// \param argc intNumber of arguments
/// \param argv[] char* List of arguments
/// \return void
///
void getPosition(int argc, char *argv[])
{
	(void) argv;
	//        If there are arguments display and error message
	if(argc > 0)
	{
		Usage("pos");
		return;
	}
	char buf[10];										//	Buffer to store the ASCII conversion of the position
	//        Else display a string stating that it is not implemented
	avrPrintf("Position = ");							//	Tag for the PC application
	//
	//	If Position has to be returned as angle carry out he value and send it to the serial port
	//
	#if (RETURN_ANGLE == 1)
	//	Carry out the angle
	double angle = 360 * ((double)Encoder.Position() / (double)Encoder.encoderMaxCount);
	//	Send it to the serial port
	avrPrintf(angle);
	#ifdef DEBUG
	avrPrintf("\nCounter= ");
	avrPrintf(ltoa(Encoder.Position(), buf, 10));
	#endif // DEBUG
	#else
	avrPrintf(ltoa(Encoder.Position(), buf, 10));		//	Send the position as circular buffer counter
	#endif
	avrPrintf(" pos OK\r\n");							//	Tag the PC application  that all is OK
}

#ifdef ENCODER_SIMULATION
	
	/**
	 *  \brief Function Wrap up to start the RTOS timer
	 *  
	 *  \return void
	 *  
	 *  \details Function Wrap up to easily start the NilRTOS timer.
	 */
	void startEncoderTimer()
	{
		nilTimer1Start(DEBUG_TIMER_INTERVAL_US);
	}	

	/**
	 *  \brief Function Wrap up to stop the RTOS timer
	 *  
	 *  \return void
	 *  
	 *  \details Function wrap-up to easily stop the NilRTOS timer
	 */
	void stopEncoderTimer()
	{
		nilTimer1Stop();
	}
	
#endif // ENCODER_SIMULATION

//////////////////////////////////////////////////////////////////////////
///
///	ISR Section
///
//////////////////////////////////////////////////////////////////////////

#ifdef ENCODER_SIMULATION 

//////////////////////////////////////////////////////////////////////////
///	DebugThread
///	This thread is only meant to simulating the encoder with a timer
///	in oder to send a pulse to the code at a given laps of time
//////////////////////////////////////////////////////////////////////////
NIL_WORKING_AREA(waDebugThread, 64);

/**
 *  \brief Encoder Debug Thread
 *  
 *  \param [in] DebugThread thread code function
 *  \param [in] arg thread code function arguments
 *  \return void
 *  
 *  \details Thread use to simulate the Encoder behaviour during debug. This thread periodically unlocks the semaphore to simulate a 
 *  pulse from an encoder. It is meant for debug only, to avoid to have an incremental encoder up and running 
 */
 NIL_THREAD(DebugThread, arg)
{
	#ifdef TIMER_DEBUG
		avrPrintf("Starting Debug Timer");		//	Plot debug message if flag is active
	#endif
	
	while(TRUE)
	{
		nilTimer1Wait();						//	Wait a new pulse from a timer
		nilSemSignal(&DebugSem);				//	Signal the new pulse to the encoder unlocking a semaphore
//		nilSemSignal(&EncoderCountSem);
		#ifdef TIMER_DEBUG
			avrPrintf("Tick");
			avrPrintf("Waiting Free");
		#endif // TIMER_DEBUG
		if (Encoder.MultiActivate)				//	Unlock a semaphore used to count a given # of pulse
		{
			nilSemSignal(&EncoderCountSem);
		}
	}
}

#else

//////////////////////////////////////////////////////////////////////////
///	If Encoder is not simulated, the pulse management is done via
///	External interrupt. 
///	For this reason ISR are herewith defined
//////////////////////////////////////////////////////////////////////////

/// \brief
/// Code for the Encoder External Interrupt ISR.
/// The code is triggered when there is an external event on the Encoder input ports
/// It checks the status of the encoder port A, B and Home to increment the encoder
/// absolute position.
/// \return void
///
void encoderISR()
{
	NIL_IRQ_PROLOGUE();									//	Required by RTOS
	#if (ENCODER_IMPLEMENTATION == A_ONLY)
		//	If A=H and B=H  Dome is turning clockwise (on the RIGHT)
		if(digitalRead(encoderA) == digitalRead(encoderB))
		{	
			// Increment encoder under lock block
			nilSysLockFromIsr();
			//if(Encoder.Position() == MAX_COUNT) Encoder.SetPosition(0);
			//else Encoder.SetPosition(Encoder.Position() + 1);
			Encoder++;
			nilSysUnlockFromIsr();
		}
		//	A=H and B=L Dome is turning anticlockwise (on the LEFT)
		else
		{		
			// Decrement encoder under lock block
			nilSysLockFromIsr();
			//if(Encoder.Position() == 0) Encoder.SetPosition(MAX_COUNT);
			//else Encoder.SetPosition(Encoder.Position() -1);
			Encoder--;
			nilSysUnlockFromIsr();
		}
	#elif (ENCODER_IMPLEMENTATION == A_AND_B)
	
	#endif // ENCODER_IMPLEMENTATION
	
	nilSemSignalI(&encoderSem);							//	Release the encoder semaphore
	if (Encoder.MultiActivate)							//	If command asked to count a #
	{													//	of pulses release the couting semaphore
		nilSemSignalI(&EncoderCountSem);
	}
	NIL_IRQ_EPILOGUE();									//	Requested by RTOS
}

/// \brief
/// Code for the Home Interrupt ISR.
/// The code is triggered when there is an external event on the Home input ports
/// It resets the increment the encoder position to 0
/// \return void
///
void homeISR()
{
	NIL_IRQ_PROLOGUE();									//	Requested by RTOS
	nilSysLockFromIsr()	;
	Encoder.SetPosition(0);								//	Set the position to 0 under lock block
	nilSysUnlockFromIsr();
	NIL_IRQ_EPILOGUE();									//	Requested by RTOS
}

#endif // not def DEBUG

//////////////////////////////////////////////////////////////////////////
///
///	Thread Section
///
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
///	Encoder Thread
///	Encoder Thread awaits locked on a semaphore that a new event (external pin/timer)
///	happens and send the new position to the PC application
//////////////////////////////////////////////////////////////////////////
NIL_WORKING_AREA(waEncoderThread, STACKSIZE);
/**
 *  \brief Encoder Thread
 *  
 *  \param [in] EncoderThread Encoder Thread code function 
 *  \param [in] arg Encoder Thread function arguments
 *  \return void
 *  
 *  \details This threads awaits that a pulse from the incremental encoder is caught locking on a semaphore.
 *  When the pulse is caught, the semaphore unlocks and the new position is carried out and sent over the
 *  serial port
 */
NIL_THREAD(EncoderThread, arg)
{
	while (TRUE)
	{
		//	If Encoder is simulated
		#ifdef ENCODER_SIMULATION 
			nilSemWait(&DebugSem);				//	Await a new event on the Debug semaphore
			nilSysLock();
			avrPrintf("Tick\n");				//	Send a tag to the PC application
			//
			//	In simulated conditions, the Dome turning state information are store by the Dome data structure
			//	Therefore it used to increment/decrement the Encoder position
			//
			if(Dome.getState() == TURN_RIGHT)
			{
				//if(Encoder.Position() == MAX_COUNT-1) Encoder.SetPosition(0);
				//else Encoder.SetPosition(Encoder.Position() + 1);
				Encoder++;	
			}
			else if(Dome.getState() == TURN_LEFT)
			{
				//if(Encoder.Position() == 0) Encoder.SetPosition(MAX_COUNT - 1);
				//else Encoder.SetPosition(Encoder.Position() - 1);
				Encoder--;
			}
			nilSysUnlock();
		#else
		//	Encoder HW is available
			nilSemWait(&encoderSem);			//	Encoder position is consumed elsewhere so here
												//	just awaits the encoder pulse to send the new position
		#endif	//	ENCODER_SIMULATION		
		char buf[10];
		avrPrintf("Position= ");				//	Tag for the PC application
		//	If the position has to be reported as angle
		#if (RETURN_ANGLE == 1)
			//	Carry out the angle
			double angle = (360 * (double)Encoder.Position()) / 16.0;//(double)ENCODER_RESOLUTION; //Encoder.encoderMaxCount);
			avrPrintf(angle);					//	Send the angle to the serial port		
			avrPrintf(CR);
		#else
		//	Position is sent as circular buffer counter
			avrPrintf(ltoa(Encoder.Position(), buf, 10));	//	Send the angle position counter on serial port
			avrPrintf(CR);
		#endif
		#ifdef MEMORY_CHECK
				avrPrintf("freeMemory() = ");		
				avrPrintf(freeMemory());
				avrPrintf(CR);
		#endif // MEMORY_CHECK
	}
}