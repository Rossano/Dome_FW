// encoder.h

#ifndef _ENCODER_h
#define _ENCODER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#define STACKSIZE	64
#define IRQ_PINA	0
#define IRQ_PINB	1
#define IRQ_HOME	2
#define MAX_COUNT	9


const uint8_t encoderA = 3;
const uint8_t encoderB = 2;
const uint8_t encoderHome = 0;


class EncoderClass
{
 private:
	unsigned long _position;
	//void encoderISR();
	//void homeISR();
	
 public:
	EncoderClass(): _position(0) {init();}
	EncoderClass(unsigned long pos /*= 0*/);
	void init();
	unsigned long Position();
	void SetPosition(unsigned long pos);
	bool channelA();
	bool channelB();
	bool channelHome();
};

extern EncoderClass Encoder;

void encoderISR();
void homeISR();
void getPosition(int argc, char *argv[]);

#endif

