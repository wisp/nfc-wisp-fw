/******************************************************************************
 * catchall.c
 *
 * @date Aug 9, 2013
 * @author anparks
 *****************************************************************************/

#include "globals.h"

uint16_t bad_interrupts;

/**
 * Catch any bad interrupts to prevent them from resetting the CPU. Keep count of all bad interrupts that occur.
 */
#pragma vector=RTC_VECTOR
#pragma vector=PORT2_VECTOR
#pragma vector=TIMER2_A1_VECTOR
#pragma vector=TIMER2_A0_VECTOR
#pragma vector=USCI_B1_VECTOR
//#pragma vector=PORT1_VECTOR
//#pragma vector=TIMER1_A1_VECTOR
//#pragma vector=TIMER1_A0_VECTOR
#pragma vector=DMA_VECTOR
#pragma vector=LDO_PWR_VECTOR
//#pragma vector=TIMER0_A1_VECTOR
#pragma vector=TIMER0_A0_VECTOR
#pragma vector=ADC10_VECTOR
#pragma vector=USCI_B0_VECTOR
#pragma vector=USCI_A0_VECTOR
#pragma vector=WDT_VECTOR
#pragma vector=TIMER0_B1_VECTOR
#pragma vector=TIMER0_B0_VECTOR
#pragma vector=COMP_B_VECTOR
#pragma vector=UNMI_VECTOR
#pragma vector=SYSNMI_VECTOR
__interrupt void CATCH_ALL(void) {
	asm(" NOP");
	bad_interrupts++;
}
