/***************************************************************************************
 * _14443_B.h
 *
 * @date May 31, 2013
 * @author Eve(Yi Zhao), Sensor Systems Lab, UW
 * 		   Derek Thrasher
 **************************************************************************************/
#ifndef _B_H
#define _B_H


//=======================================================================================
//									Includes
//=======================================================================================
#include "../common/globals.h"

//for 6.78Mhz clock
#ifdef _15693_1of256
#define TIME_OUT 90
#else
#define TIME_OUT 196
#endif


//for NFC_rev1.0, the SOF is 45us
//#define SOF 115
//#define Dt		   8    //10/time tolerance +/-
#define Dt		   10        //10/time tolerance +/-
#define zDt		   8		//extra tolerance for zero
#define ErrDt	   15       //Dt+Dt/2
#define SOB		   11		//start bit
#define ONE_PULSES 13   	//7.6us~11.2us 16-/+3
#define TWO_PULSES 29		//17.1~20.6us 32-/+3
#define THREE_PULSES 45		//26.54~30.08us 48-/+3
#define FOUR_PULSES  61 	//35.98~39.08us 64-/+3
#define FIVE_PULSES 77  	//45.42~48.9us 80-/+3
#define SIX_PULSES 93  	    //54.86~58.4us 96-/+3
#define SEVEN_PULSES 109  	//64.3~67.8.9us 112-/+3
#define EIGHT_PULSES 125  	//73.7~77.2us 128-/+3
#define NINE_PULSES 141 	//83.1~86.7us 144-/+3
#define TEN_PULSES  157 	//92.6~96.16us 160-/+3
#define ELEVEN_PULSES 173 	//102.0~105.6us 176-/+3
#define SOF 19				//@note: from 22 to 8 at leaset 18us should change depending differenct range
#define TIMEOUT ELEVEN_PULSES
#define EOF	175
//#define SOF 180;

//=======================================================================================
//									Micros
//=======================================================================================
#define Enable_Rx() \
	RX_ENABLE_OUT |= RX_ENABLE_BIT; \
	RX_ENABLE_DIR |= RX_ENABLE_BIT; \
	RX_IES &= ~RX_BIT;  //detect rising edge

#define Disable_Rx() \
	RX_ENABLE_DIR &= ~RX_ENABLE_BIT; \
	RX_IE &= ~RX_BIT; \
	RX_IFG &= ~RX_BIT;
	//RX_ENABLE_DIR &= ~RX_ENABLE_BIT; \
	//RX_ENABLE_OUT &= ~RX_ENABLE_BIT; \

//=======================================================================================
//									External variable
//=======================================================================================
//extern uint8_t _14443_buf_ptr = 0;
extern uint8_t validRx;
extern uint8_t rx_buffer[CMD_BUF_SIZE];
extern uint8_t _14443_buf_ptr;
extern uint8_t volatile doNFC_state;

//=======================================================================================
//									External functions
//=======================================================================================
unsigned short doNFC_Rx(void);


#endif /* 14443_B_H_ */
