/******************************************************************************
 * send_bpsk.h
 *
 * @date June 5, 2013
 * @author Derek Thrasher (UMass Amherst)
 ******************************************************************************/

#ifndef SEND_BPSK_H_
#define SEND_BPSK_H_

#include "globals.h"

//6.78Mhz where 1etu = 8clock pulses
// TODO Get rid of ifdefs if possible
#ifdef _6_78Mhz
#define ONE_ETU 8
#define TWO_ETU 16
#define TEN_ETU 80
#else
#define ONE_ETU 16
#define TWO_ETU 32
#define TEN_ETU 164//160
#endif
//assembly constants used for send routine
asm("SEND_BIT_MASK .set R6");
asm("SEND_COMMAND .set R7");
//asm("SEND_BIT_SIZE .set R6");
asm("SEND_CURRENT_WORD .set R8");
//asm("SEND_BIT_COUNT .set R8");

asm("TBCTL_ADR .set 0180h");			//timer control address
asm("ASM_MC_1 .set 0010h");				//continuous mode bits

//=============================================================================
//									Macros
//=============================================================================
#define ENABLE_PWM()	TB0CTL |= MC_3;		//enable pwm up/down mode
#define DISABLE_PWM()	TB0CTL &= ~MC_3;	//disable


//********************************************************************************
//							Protocol Constants
//********************************************************************************
// TODO Get rid of ifdefs if possible
#ifdef _6_78Mhz
#define POST_RX_WAIT	64 //75.56 us@ 6.78 MHz / 8
#else//13.56Mhz
#define POST_RX_WAIT	124//128 //75.56 us@ 13.56 MHz / 8
#endif

extern uint8_t transmitCommand[CMD_BUF_SIZE];


void initialize_bpsk(void);
void send_bpsk(void);
void delay(uint16_t);
void shift_mask();
uint8_t next_bit();

#endif /* SEND_LDR_H_ */
