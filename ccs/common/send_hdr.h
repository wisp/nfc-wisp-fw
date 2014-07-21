/*
 * send_hdr.h
 *
 *  Created on: Mar 8, 2013
 *      Author: Jeremy Gummeson
 */

#ifndef SEND_LDR_H_
#define SEND_LDR_H_

#include "globals.h"


//assembly constants used for send routine
asm("SEND_BIT_MASK .set R4");
asm("SEND_COMMAND .set R5");
asm("SEND_BIT_SIZE .set R6");
asm("SEND_CURRENT_WORD .set R7");
asm("SEND_BIT_COUNT .set R8");

asm("TBCTL_ADR .set 0180h");			//timer control address
asm("ASM_MC_1 .set 0010h");				//continuous mode bits

//=============================================================================
//									Macros
//=============================================================================
#define ENABLE_PWM()	TB0CTL |= MC_3;//asm(" BIS #ASM_MC_1, &TBCTL_ADR"); //asm(" BIS #0040h, &0186h"); asm(" NOP");//enable pwm
#define DISABLE_PWM()	TB0CTL &= ~MC_3;//asm(" BIC #ASM_MC_1, &TBCTL_ADR");  //asm(" BIS #0040h, &0186h");//asm(" MOV &TBR_ADR, R8"); //code does not work


//********************************************************************************
//							Protocol Constants
//********************************************************************************
#define POST_RX_WAIT	510 //300 us@ 13.56 MHz / 8
#define LONG_TX		 	96 //56.64 us @ 13.56 MHz / 8
#define HALF_TX_PERIOD	31 //18.88 us @ 13.56 MHz / 8

void initialize_hdr(void);
void send(void);
void delay(uint16_t);

#endif /* SEND_LDR_H_ */
