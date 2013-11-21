// 
//	Encoder.cpp 
// 

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

static SEMAPHORE_DECL(EncoderCountSem, 0);	//	Semaphore used when counting fixed # of steps

//////////////////////////////////////////////////////////////////////////
///
///	Code Section
///
//////////////////////////////////////////////////////////////////////////

extern void avrPrintf(const char * str);	//	function to print to terminal a string

///<summary>
///	Default Constructor
///</summary>
EncoderClass::EncoderClass()
{
	_position = 0;
	init();
}

/// <summary>
/// Constructor Initializing the Encoder position.
/// </summary>
/// <param name="pos">Initial position of the encoder</param>
EncoderClass::EncoderClass(uint32_t pos)
{
	_position = pos;
	init();		
}

/// <summary>
/// Initialize the Encoder HW and EncoderClass data structure.
/// </summary>
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
		attachInterrupt(IRQ_PINB, encoderISR, RISING);
		attachInterrupt(IRQ_HOME, homeISR, RISING);
	#endif //ENCODER_SIMULATION		
}

/// <summary>
/// Method to read the Encoder position.
/// </summary>
/// <return name="uint32_t">Encoder actual absolute position.</return>
uint32_t EncoderClass::Position()
{
	return _position;
}

/// <summary>
/// Method to Set the Encoder position.
/// </summary>
/// <param name="pos">New position of the encoder</param>
void EncoderClass::SetPosition(uint32_t pos)
{
	//	Since it is a critical parameter better to updated under a lock block
	nilSysLock();
	_position = pos;
	nilSysUnlock();
}

/// <summary>
/// Method to read the Encoder A signal state.
/// </summary>
/// <return name="bool">Encoder A channel state.</return>
bool EncoderClass::channelA()
{
	return digitalRead(encoderA);
}

/// <summary>
/// Method to read the Encoder B signal state.
/// </summary>
/// <return name="bool">Encoder B channel state.</return>
bool EncoderClass::channelB()
{
	return digitalRead(encoderB);
}

/// <summary>
/// Method to read the HOME signal state.
/// </summary>
/// <return name="bool">HOME signal state.</return>
bool EncoderClass::channelHome()
{
	return digitalRead(encoderHome);
}

//
//	Definition of operators ++ & -- to change the Encoder position in a more elegant way
//

/// <summary>
/// Operator ++.
/// </summary>
/// <return name="EncoderClass&">New EncoderClass data structure.</return>
EncoderClass& EncoderClass::operator ++()
{
	return *this;
}

/// <summary>
/// Operator ++..
/// </summary>
/// <return name="EncoderClass">EncoderClass incremented.</return>
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

/// <summary>
/// Operator --.
/// </summary>
/// <return name="EncoderClass&">New EncoderClass data structure.</return>
EncoderClass& EncoderClass::operator --()
{
	return *this;
}

/// <summary>
/// Operator --..
/// </summary>
/// <return name="EncoderClass">EncoderClass decremented.</return>
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

#ifdef ENCODER_SIMULATION //DEBUG

NIL_WORKING_AREA(waDebugThread, 64);
NIL_THREAD(DebugThread, arg)
{
	#ifdef TIMER_DEBUG
	//Serial.println("Starting Debug Timer");
	avrPrintf("Starting Debug Timer");
	#endif
	//nilTimer1Start(DEBUG_TIMER_INTERVAL_US);
	
	while(TRUE)
	{
		nilTimer1Wait();
		nilSemSignal(&DebugSem);
//		nilSemSignal(&EncoderCountSem);
		#ifdef TIMER_DEBUG
		//Serial.println("Tick!\nWaiting Free");
		avrPrintf("Tick");
		avrPrintf("Waiting Free");
		#endif // TIMER_DEBUG
		if (Encoder.MultiActivate)
		{
			nilSemSignal(&EncoderCountSem);
		}
	}
}

#else

void encoderISR()
{
	NIL_IRQ_PROLOGUE();
	if(digitalRead(encoderA) == digitalRead(encoderB))
	{
		nilSysLockFromIsr();
		if(Encoder.Position() == MAX_COUNT) Encoder.SetPosition(0);
		else Encoder.SetPosition(Encoder.Position() + 1);
		nilSysUnlockFromIsr();
	}
	else
	{
		nilSysLockFromIsr();
		if(Encoder.Position() == 0) Encoder.SetPosition(MAX_COUNT);
		else Encoder.SetPosition(Encoder.Position() -1);
		nilSysUnlockFromIsr();
	}
	nilSemSignalI(&encoderSem);
	if (Encoder.MultiActivate)
	{
		nilSemSignalI(&EncoderCountSem);
	}
	NIL_IRQ_EPILOGUE();
}

void homeISR()
{
	NIL_IRQ_PROLOGUE();
	nilSysLockFromIsr()	;
	Encoder.SetPosition(0);
	nilSysUnlockFromIsr();
	NIL_IRQ_EPILOGUE();
}

#endif // not def DEBUG

void getPosition(int argc, char *argv[])
{
	(void) argv;
	//        If there are arguments display and error message
	if(argc > 0)
	{
		Usage("pos");
		return;
	}
	char buf[10];
	//        Else display a string stating that it is not implemented
	avrPrintf("Position= ");
	#if (RETURN_ANGLE == 1)
		double angle = 360 * ((double)Encoder.Position() / (double)Encoder.encoderMaxCount);
		avrPrintf(angle);
		avrPrintf("\nCounter= ");
		avrPrintf(ltoa(Encoder.Position(), buf, 10));
	#else
		avrPrintf(ltoa(Encoder.Position(), buf, 10));
	#endif
	avrPrintf("OK\r\n");
}
#include "MemoryFree.h"

NIL_WORKING_AREA(waEncoderThread, STACKSIZE);
NIL_THREAD(EncoderThread, arg)
{
	while (TRUE)
	{
		#ifdef ENCODER_SIMULATION 
			nilSemWait(&DebugSem);
			nilSysLock();
			avrPrintf("Tick\n");
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
			nilSemWait(&encoderSem);
		#endif	//	ENCODER_SIMULATION
		//Serial.print("ARD> Position -> ");
		//Serial.println(Encoder.Position());
		char buf[10];
		avrPrintf("Position= ");
		#if (RETURN_ANGLE == 1)
			double angle = (360 * (double)Encoder.Position()) / 16.0;//(double)ENCODER_RESOLUTION; //Encoder.encoderMaxCount);
			avrPrintf(angle);			
			avrPrintf(CR);
		#else
			avrPrintf(ltoa(Encoder.Position(), buf, 10));
		#endif
		#ifdef MEMORY_CHECK
				avrPrintf("freeMemory() = ");		
				avrPrintf(freeMemory());
				avrPrintf(CR);
		#endif // MEMORY_CHECK
	}
}

void startEncoderTimer()
{
	nilTimer1Start(DEBUG_TIMER_INTERVAL_US);
}

void stopEncoderTimer()
{
	nilTimer1Stop();
}