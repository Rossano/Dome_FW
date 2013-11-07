/*
 * board.h
 *
 * Created: 04/11/2013 17:38:41
 *  Author: Ross
 */ 


#ifndef BOARD_H_
#define BOARD_H_

///<summary>
///	Link to AVR Inner IRQ 
///</summary>
#define IRQ_PINA	0
#define IRQ_PINB	1
#define IRQ_HOME	2
//#define MAX_COUNT	9

/// <summary>
/// Incremental Enc oder Arduino interface Definition.
/// </summary>
const uint8_t encoderA = 3;
const uint8_t encoderB = 2;
const uint8_t encoderHome = 0;

/// <summary>
/// Motor Control (Dome) Arduino interface Definition.
/// </summary>
const uint8_t turnLeftPin = 8;
const uint8_t turnRightPin = 9;

#endif /* BOARD_H_ */