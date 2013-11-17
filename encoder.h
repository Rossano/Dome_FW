// encoder.h

#ifndef _ENCODER_h
#define _ENCODER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

//#ifdef DEBUG
//#include "debug.h"
//#endif // DEBUG

#define STACKSIZE	64
//#define IRQ_PINA	0
//#define IRQ_PINB	1
//#define IRQ_HOME	2
#define ENCODER_RESOLUTION	16
#define ENCODER_GEAR_RATIO  1.0
#define ENCODER_MAX_COUNT	floor(ENCODER_RESOLUTION * ENCODER_GEAR_RATIO)
#define MAX_COUNT	9
///		Activate the simulation of the encoder
#define ENCODER_SIMULATION
#define RETURN_ANGLE		1

//extern float gearRatio;
//extern uint16_t encoderResolution;
//extern float gearRation = 1.0;
//extern uint32_t encoderMaxCount;

//const uint8_t encoderA = 3;
//const uint8_t encoderB = 2;
//const uint8_t encoderHome = 0;

class EncoderClass
{
 public:
	uint16_t encoderResolution;
	double gearRatio;
	uint32_t encoderMaxCount;
 private:
	uint32_t _position;
	//void encoderISR();
	//void homeISR();
	
 public:
	EncoderClass();//: _position(0) {init();}
	EncoderClass(uint32_t pos /*= 0*/);
	void init();
	uint32_t Position();
	void SetPosition(unsigned long pos);
	bool channelA();
	bool channelB();
	bool channelHome();
	EncoderClass& operator++();
	EncoderClass operator++(int);
	EncoderClass& operator--();
	EncoderClass operator--(int);
	bool MultiActivate;
};

extern EncoderClass Encoder;

//SEMAPHORE_DECL(EncoderCountSem, 0);

void encoderISR();
void homeISR();
void getPosition(int argc, char *argv[]);

//////////////////////////////////////////////////////////////////////////
///	Function Wraps-up to start/stop the encoder timer 
///	(to avoid to export too many data structures)
//////////////////////////////////////////////////////////////////////////
#ifdef ENCODER_SIMULATION
	void startEncoderTimer();
	void stopEncoderTimer();
#endif // ENCODER_SIMULATION

#endif

