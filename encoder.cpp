// 
// 
// 

#include <NilRTOS.h>

#include "board.h"
#include "encoder.h"
#include "shell.h"

//#define DEBUG
#define ENCODER_SIMULATION
#define DEBUG_TIMER_INTERVAL_US		3000000
#undef TIMER_DEBUG

#ifdef ENCODER_SIMULATION 
#include <NilTimer1.h>
//#include "debug.h"
#endif // ENCODER_SIMULATION

EncoderClass Encoder;

float gearRation = 1.0;

#ifdef ENCODER_SIMULATION 
//extern SEMAPHORE_DECL(DebugSem, 0);
	#include "dome.h"
	SEMAPHORE_DECL(DebugSem, 0);
	extern DomeClass Dome;
#else
	SEMAPHORE_DECL(encoderSem, 0);
#endif // ENCODER_SIMULATION

static SEMAPHORE_DECL(EncoderCountSem, 0);

extern void avrPrintf(const char * str);

//EncoderClass::EncoderClass()
//{
	//_position = 0;
	//init();
//}

EncoderClass::EncoderClass(unsigned long pos /* = 0 */)
{
	_position = pos;
	init();		
}

void EncoderClass::init()
{	
	MultiActivate = false;
	pinMode(encoderA, INPUT_PULLUP);
	pinMode(encoderB, INPUT_PULLUP);
	pinMode(encoderHome, INPUT_PULLUP);
	digitalWrite(encoderA, HIGH);
	digitalWrite(encoderB, HIGH);
	digitalWrite(encoderHome, HIGH);
	#ifdef ENCODER_SIMULATION 
	///	DO NOTHING
		//nilTimer1Start(DEBUG_TIMER_INTERVAL_US);
	#else	
		attachInterrupt(IRQ_PINA, encoderISR, RISING);
		attachInterrupt(IRQ_PINB, encoderISR, RISING);
		attachInterrupt(IRQ_HOME, homeISR, RISING);
	#endif //ENCODER_SIMULATION		
}

unsigned long EncoderClass::Position()
{
	return _position;
}

void EncoderClass::SetPosition(unsigned long pos)
{
	nilSysLock();
	_position = pos;
	nilSysUnlock();
}

bool EncoderClass::channelA()
{
	return digitalRead(encoderA);
}

bool EncoderClass::channelB()
{
	return digitalRead(encoderB);
}

bool EncoderClass::channelHome()
{
	return digitalRead(encoderHome);
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
		nilSemSignal(&EncoderCountSem);
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
	avrPrintf("Position -> ");
	avrPrintf(ltoa(Encoder.Position(), buf, 10));
	avrPrintf("OK\r\n");
}

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
				if(Encoder.Position() == MAX_COUNT-1) Encoder.SetPosition(0);
				else Encoder.SetPosition(Encoder.Position() + 1);	
			}
			else if(Dome.getState() == TURN_LEFT)
			{
				if(Encoder.Position() == 0) Encoder.SetPosition(MAX_COUNT - 1);
				else Encoder.SetPosition(Encoder.Position() - 1);
			}
			nilSysUnlock();
		#else
			nilSemWait(&encoderSem);
		#endif	//	ENCODER_SIMULATION
		//Serial.print("ARD> Position -> ");
		//Serial.println(Encoder.Position());
		char buf[10];
		avrPrintf("Position -> ");
		avrPrintf(ltoa(Encoder.Position(), buf, 10));
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