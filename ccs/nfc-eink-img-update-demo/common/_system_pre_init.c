/*****************************************************************************
 * @file    NFC_WISP_Rev1.0.c
 * @fun     MSP430 _system_pre_init() and Interrupt Routine 
 * @date    10/25/2014
 * @author  Eve (Yi Zhao), Sensor systems Lab. UW
 * @notes   note that the system_pre_init and interrupt vectors do not need \
 * 			definitions in the header files. This is because
 * 				compiler automatically links them.
 */
/******************************************************************************/


//=============================================================================
//									Includes
//=============================================================================
	#include "globals.h"
	uint16_t bad_interrupts;
	//uint8_t isDoingLowPwrSleep;

//=============================================================================
//									Functions
//=============================================================================
/**
 * 	@fcn		void _system_pre_init (void)
 *  @brief		routine called by boot() right before main() is called.
 *  @pre		c_int00 was just called after a POR/PUC event
 *  @post		the RAM, clock and peripherals are initialized for use
 *	@note		ACLK,FLL is disabled in _system_pre_init, \
 *				UCSCTL8 used to clear SMCLKREQEN,MCLKREQEN,ACLKREQEN \
 *				if do not want in LPM
 */
int _system_pre_init (void) {

	//--------------------Setup Watchdog, IO----------------------------------//
	WDTCTL = WDTPW + WDTHOLD;		/* disable the watchdog timer			-*/
	setupDflt_IO();
	_bic_SR_register(GIE); 			//Turn off GIE while we initialize.
	bad_interrupts = 0;

	//Turn LDO off
	LDOKEYPID = LDOKEY;
	LDOPWRCTL = 0x00;
	LDOKEYPID = 0;

	// Turn off high/low voltage supervisors
	PMMCTL0_H = PMMPW_H;
	SVSMHCTL &= ~(SVMHE+SVSHE);
	SVSMLCTL &= ~(SVMLE+SVSLE);
	PMMCTL0_H = 0x00; 		

	//---------------------------Setup Clock----------------------------------//

	// ACLK=REFO,SMCLK=DCO,MCLK=DCO During initialization
	UCSCTL4 |= SELA__REFOCLK + SELM__DCOCLK + SELS__DCOCLK;
	//turn off the FLL and ACLK since we do not use it at the very beginning
	//_BIS_SR(SCG0+OSCOFF);			//@note: must keep ACLK on for timeout system

	UCSCTL6 &= ~XT2OFF; // Turn on XT2OFF
	//UCSCTL4 |= SELA_2; // ACLK=REFO,SMCLK=DCO,MCLK=DCO During initialization
	/*Loop until XT1,XT2 & DCO stabilizes - in this case loop until XT2 settles*/
	do
	{
		UCSCTL7 &= ~(XT2OFFG | XT1LFOFFG | DCOFFG);		// Clear XT2,XT1,DCO fault flags
		SFRIFG1 &= ~OFIFG;								// Clear fault flags
	}while (SFRIFG1&OFIFG);								// Test oscillator fault flag

	UCSCTL6 &= ~XT2DRIVE1;								// Decrease XT2 Drive according to expected frequency
	UCSCTL4 |= SELS_5 | SELM_5 | SELA_0;				// SMCLK=MCLK=XT2, ACLK=XT1,
	UCSCTL7 = 0; 										//clear fault flags
	return 0;
}




/**
 * Catch any bad interrupts to prevent them from resetting the CPU.
 * Keep count of all bad interrupts that occur.
 */
#pragma vector=RTC_VECTOR
#pragma vector=PORT2_VECTOR
#pragma vector=TIMER2_A1_VECTOR
#pragma vector=  USCI_A1_VECTOR
//#pragma vector=TIMER2_A0_VECTOR
//#pragma vector=USCI_B1_VECTOR
//#pragma vector=PORT1_VECTOR
#pragma vector=TIMER1_A1_VECTOR
//#pragma vector=TIMER1_A0_VECTOR
#pragma vector=DMA_VECTOR
#pragma vector=LDO_PWR_VECTOR
#pragma vector=TIMER0_A1_VECTOR
//#pragma vector=TIMER0_A0_VECTOR
//#pragma vector=ADC10_VECTOR
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
