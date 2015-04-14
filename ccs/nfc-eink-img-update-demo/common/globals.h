/************************************************************************************************************************************/
/** @file		globals.h
 * 	@brief		NFC global defines and funcs
 * 	@author		Yi Zhao(Eve), UW Sensor Systems Lab
 * 	@created	8.15.2014
 */
/************************************************************************************************************************************/
#ifndef GLOBALS_H_
#define GLOBALS_H_

//=============================================================================
//									Includes
//=============================================================================
#include "NFC_WISP_Rev1.0.h"
#include  "spi.h"
#include <msp430f5310.h>
#include <stdint.h>
#include <stdbool.h>
#include "timer.h"
//#include "e-paper.h"
//=============================================================================
//									Configuration
//=============================================================================
/*******SELECT PROTOCOL*************/
//#define _15693_1of256
#define _14443_B

/******SELECT CLOCK FREQUENCY******/
//#define _6_78MHz
#define _13_56MHz

//=============================================================================
//									General Defines
//=============================================================================
#define CMD_BUF_SIZE 64
//#define CMD_BUF_SIZE 1
//@modify only for RFID 2015
#ifdef TempDemo
		#define UID_SIZE 8
#else
		#define UID_SIZE 4
#endif

#define APP_SIZE 4
#define CAPTURE_BUFFER_SIZE	10  //maximum number of pulse capture times we can store

#define CRYSTAL_FREQ	32768
#define TIMEOUT_MS		100   //low power timeout in ms
#define TIMEOUT_VAL		(CRYSTAL_FREQ/(TIMEOUT/10))






#define LED_1_PULSE_FLAG	0x01
#define LED_2_BIT_PULSE_FLAG	0x02
#define KILL_LEDS_FLAG		0x80


#define	FOREVER 		(1)
#define NEVER 			(0)

#define TRUE  			(1)
#define FALSE 			(0)

#define HIGH			(1)
#define	LOW				(0)
//#define NULL			(void *)0  /**< define NULL */

#define FAIL			(0)
#define SUCCESS			(1)


//=============================================================================
//									NFC Defines
//=============================================================================
//6.78Mhz where 1 etu is clock periods
// TODO Get rid of ifdefs if possible
#ifdef _14443_B

/****************state for NFC state machine **********************************/
//Start NFC routine after first power up or sleep_untill_read
//@note Change below bits setting have to change TimerA0 ISR in timer.c
#define NFC_Stop		0		// NFC halt
#define NFC_Start       BIT7	// doNFC_state = BIT7
#define NFC_Rx0			BIT0	// doNFC_state =BIT0 detect delimeter start
#define NFC_Rx1			BIT1	// doNFC_state =BIT1 detect start of first byte start
#define NFC_Rx2			BIT2	// doNFC_state =BIT2 decode data
#define NFC_Tx			0x07	// doNFC_state =0x07  transmit data
#define NFC_Sleep		BIT3	// doNFC_state = BIT3
#define NFC_LPM4		BIT4	// doNFC_state is in LMP4 now



#ifdef _6_78MHz // 6.78 MHz clock frequency

#define TEN_PULSES 70//80
#define ELEVEN_PULSES 98//88
#define EOF 86

#else // Otherwise assume 13.56Mhz/8 clock frequency
//threshold should be 70us
#endif /* _6_78MHz */
#endif /* _14443_B */


//=============================================================================
//									Globals
//=============================================================================
/***************NFC Globals****************************/
extern uint8_t	volatile 	doNFC_state;
//extern uint8_t transmitCommand[CMD_BUF_SIZE];
/************E-ink Update state************************/
//extern uint8_t 				imageBuffer[E_INK_SIZE];
//extern uint8_t volatile 	imageUpdateState;
//***********Sensing Update state**********************/
extern volatile unsigned long RTC_ctr;
extern volatile uint8_t		senseState;
extern volatile uint8_t		accelState;

/****Sense data cicular queue structure***************/
extern uint8_t	 	buf_full;
extern uint8_t      led_pulse_flags; // Flags indicating which LED to pulse in the timer ISR
//=============================================================================
//									Functions
//=============================================================================
void led_1_auto_pulse();
void LED_2_BIT_auto_pulse();

//=============================================================================
//									Macros
//=============================================================================
// LED Macros
#define toggle_led_1() LED_1_OUT ^= LED_1_BIT
#define toggle_led_2() LED_2_OUT ^= LED_2_BIT
#define led_1_off() LED_1_OUT &= ~(LED_1_BIT)
#define led_2_off() LED_2_OUT &= ~(LED_2_BIT)
#define led_1_on() LED_1_OUT |= LED_1_BIT
#define led_2_on() LED_2_OUT |= LED_2_BIT
#endif
