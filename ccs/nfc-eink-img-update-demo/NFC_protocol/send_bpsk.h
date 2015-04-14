/******************************************************************************
 * send_bpsk.h
 *
 * @date June 5, 2013
 * @author Derek Thrasher (UMass Amherst)
 ******************************************************************************/

#ifndef SEND_BPSK_H_
#define SEND_BPSK_H_

#include "../common/globals.h"
//#include "../common/NFC_WISP_Rev1.0.h"

//6.78Mhz where 1etu = 8clock pulses
// TODO Get rid of ifdefs if possible
#ifdef _6_78Mhz
#define ONE_ETU 8
#define TWO_ETU 16
#define TEN_ETU 80
#else
#define ONE_ETU 16
#define TWO_ETU 32
#define TWO_ETU_SLEEP 22
#define TEN_ETU 160//160
#define TEN_ETU_SLEEP 150
#endif
//assembly constants used for send routine
asm("SEND_BIT_MASK .set R6");
asm("SEND_COMMAND .set R7");
//asm("SEND_BIT_SIZE .set R6");
asm("SEND_CURRENT_WORD .set R8");
asm("SEND_BIT .set R12");
//asm("SEND_BIT_COUNT .set R8");

asm("TBCTL_ADR .set 0180h");			//timer control address
asm("ASM_MC_1 .set 0010h");				//continuous mode bits

//=============================================================================
//									Macros
//=============================================================================
//TB0CCR0 = 8; 			//upper bound compare
//TB0CCR2 = 0;			//lower bound compare
//ensures that we will get a rising edge imediately
#define ENABLE_PWM() \
						TB0R = 7; \
						TB0CCTL2 =	OUTMOD_3; \
						TB0CTL = TBSSEL_2+MC_3; //SMCLK 13.56Mhz,pwm up/down mode


#define DISABLE_PWM() TB0CTL = 0;	//disable


//********************************************************************************
//							Protocol Constants
//********************************************************************************
// TODO Get rid of ifdefs if possible
#ifdef _6_78Mhz
#define POST_RX_WAIT	64 //75.56 us@ 6.78 MHz / 8
#else//13.56Mhz
#define POST_RX_WAIT	128//128 //75.56 us@ 13.56 MHz / 8
#endif

//extern uint8_t transmitCommand[CMD_BUF_SIZE];
void initialize_bpsk(void);
void send_bpsk(void);
void delay(uint16_t);
void shift_mask();
uint8_t next_bit();
//extern uint8_t transmitCommand[CMD_BUF_SIZE];
#endif /* SEND_LDR_H_ */
