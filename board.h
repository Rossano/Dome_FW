/*
 * board.h
 *
 * Created: 04/11/2013 17:38:41
 *  Author: Ross
 */

//////////////////////////////////////////////////////////////////////////
///
///		Board Pin Mapping for the Dome controller board
///
//////////////////////////////////////////////////////////////////////////

#ifndef BOARD_H_
#define BOARD_H_

/** \brief Link to AVR Inner IRQ
 *
 * \param IRQ_PINA Linked to IRQ0
 * \param IRQ_PINB Linked to IRQ1
 * \param IRQ_HOME Linekd to IRQ2
 *
 */
#define IRQ_PINA	0
#define IRQ_PINB	1
#define IRQ_HOME	2
//#define MAX_COUNT	9

/** \brief Incremental Enc oder Arduino interface Definition.
 *
 * \param encoderA uint8_t Encoder A signal on PIN3
 * \param encoderB uint8_t Encoder B signal on PIN2
 * \param encoderHome uint8_t Home signal on PIN0
 *
 */
const uint8_t encoderA = 2;
const uint8_t encoderB = 3;
const uint8_t encoderHome = 0;

/** \brief Motor Control (Dome) Arduino interface Definition
 *
 * \param turnLeftPin uint8_t Signal to turn left on PIN8
 * \param turnRightPin uint8_t Signal to turn right on PIN9
 *
 */
const uint8_t turnLeftPin = 8;
const uint8_t turnRightPin = 9;

/** \brief Encoder Generator (Dome)  Definition
 *
 * \param encoderA uint8_t Signal  on PIN4
 * \param encoderB uint8_t Signal  on PIN5
 *
 */
const uint8_t SimencoderA = 4;
const uint8_t SimencoderB = 5;

#endif /* BOARD_H_ */
