// 
// 
// 

#include <NilRTOS.h>

#include "encoder.h"
#include "shell.h"

#define DEBUG

#ifdef DEBUG
#include "debug.h"
#endif // DEBUG

EncoderClass Encoder;

SEMAPHORE_DECL(encoderSem, 0);

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
	pinMode(encoderA, INPUT_PULLUP);
	pinMode(encoderB, INPUT_PULLUP);
	pinMode(encoderHome, INPUT_PULLUP);
	digitalWrite(encoderA, HIGH);
	digitalWrite(encoderB, HIGH);
	digitalWrite(encoderHome, HIGH);
	
	attachInterrupt(IRQ_PINA, encoderISR, RISING);
	attachInterrupt(IRQ_PINB, encoderISR, RISING);
	attachInterrupt(IRQ_HOME, homeISR, RISING);
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
	#ifdef DEBUG
		nilSemWait(&DebugSem);
	#else
		nilSemWait(&encoderSem);
	#endif
		//Serial.print("ARD> Position -> ");
		//Serial.println(Encoder.Position());
		char buf[10];
		avrPrintf("Position -> ");
		avrPrintf(ltoa(Encoder.Position(), buf, 10));
	}
}
