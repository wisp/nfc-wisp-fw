/*
 * timer.h
 *
 *  Created on: Aug 26, 2014
 *      Author: evezhao
 */

#ifndef TIMER_H_
#define TIMER_H_


#include <msp430.h>
#include <stdint.h>
#include "globals.h"

/*-----------------------Low Freq LowPower TIME OUT DELAY--------------------*/
#define		RTC_15s					0xF000//15s when senseDelay is 1
#define		RTC_5s					0x5000//5s  when senseDelay is 1
#define 	RTC_2s					0x2000
#define 	RTC_3s					0x3000

#define 	senseDelay				1//4  //Set to larger value if we need more delay
#define 	TIMEOUT_COUNTS 			100
#define		TIMEOUT_10MS			328
#define		TIMEOUT_20MS			654
#define		TIMEOUT_5MS				164
#define		TIMEOUT_240uS			8
#define		lowPowerSleep_30us		2
#define		lowPowerSleep_10MS		333
#define		lowPowerSleep_5MS		165
#define 	lowPowerSleep_1MS		33

/*-----------------------High Freq CLCK LowPower DELAY--------------------*/
/**********************************
 * Constant used in delay_us()
 * num of 13.56Mhz/8 cycles
**********************************/
#define T_5us		0x0A
#define T_10us      18
#define T_25us		0x2C
#define T_30us		0x34
#define T_35us		64
#define T_40us		T_10us*4
#define T_50us 		90
#define T_100us 	T_10us*10
#define T_500us		T_100us*5
/*
#define T_1ms		0x6A4
#define T_5ms		0x2134
#define T_10ms		0x4268
#define T_25ms		0xA604
#define T_20ms		0x84D0
*/
/**********************************
 * Constant used in long_delay_ms()
 * num of T_20ms cycles
 **********************************/
/*
#define T_40ms 		0x02
#define T_120ms		0x06
#define T_240ms 	0x0C
*/
/**********************************
 * Constant used in lowPowerSleep()
 * for 32K CLK
 **********************************/
#define LPM_50us	0x3
#define LPM_500us	0x12
#define LPM_1ms		0x23
#define LPM_3ms		LPM_1ms*3
#define LPM_4ms		LPM_1ms*4
#define LPM_5ms		0xA5
#define LPM_10ms	LPM_5ms*2
#define LPM_20ms	0x294
#define LPM_25ms	LPM_5ms*5
#define LPM_40ms	0x520
#define LPM_80ms	0x520*2
#define LPM_120ms	LPM_40ms*3
#define LPM_240ms	LPM_40ms*6




//extern unsigned long timeout_ctr;
//extern uint8_t led_pulse_flags;					 // Flags indicating which LED to pulse in the timer ISR
//extern uint8_t isDoingLowPwrSleep;
extern uint8_t volatile	doNFC_state;  				 //Flags used for TimerA0 when receive command
extern uint8_t volatile imageUpdateState;
unsigned int timeout_occurred(void) ;
void timeout_init(uint16_t time) ;
void lowPowerSleep (uint16_t duration);
void long_lowPowerSleep (uint16_t duration);
void timeout_halt(void);
void delay_us(uint16_t delay);
void initRTC(uint16_t time);
void ACLK_on(void);
#endif /* TIMER_H_ */
