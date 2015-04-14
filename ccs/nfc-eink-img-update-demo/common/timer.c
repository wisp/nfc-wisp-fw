/*
 * timer.c
 *
 *  Created on: Aug 26, 2014
 *      Author: evezhao
 *      @fun timeout and lowPowerSleep timming
 */

#include "timer.h"
#include "e-paper.h"


/******************************************************************************
 * @brief	TA0 Timeout system for monitor timeout such as in sleep_untill_edges
 * @CLK     32K/8=4K, 250us/cycles
 *****************************************************************************/
void timeout_init(uint16_t time) {
	// Set up TA1.0 for periodic interrupt
	//_BIC_SR(OSCOFF);							// ENABLE ACLK
	TA0CCR0 = time; 							// 10 ms with 32768 Hz ACLK
	TA0CTL = TASSEL_1 | ID_3 | MC_1 | TACLR;	//ACLK, upmode, 4khz clk
	TA0CCTL0 = CCIE; 							//Enable interrupt
}

//Halt the timeout system (e.g., to save power)
void timeout_halt(void) {
	TA0CTL = 0x00;
	TA0CCTL0 = 0;
	//_BIS_SR(OSCOFF);
}

/******************************************************************************
 * init real time clk count for every 15s for tempSense
 *****************************************************************************/
void initRTC(uint16_t time){
	//_BIC_SR(OSCOFF);							//Enable ACLK
	TA1CCR0 = time; 							// 10 ms with 32768 Hz ACLK
	//TA1CCR0 = 0xFFFE;
	TA1CTL = TASSEL_1 | ID_3 |  MC__UP | TACLR;	//ACLK, upmode, 4khz clk
	TA1CCTL0 |= CCIE;
	RTC_ctr = 0;								//couting is 0;
	//senseState = 0;								//
}

/*******************************************************************************/
/*  @brief	sleep the CPU for duration(us~ms) using TimerA2
 *  @note	must use when ACLKREQEN=1
*******************************************************************************/
//low power and more accurate sleep delay
//SMCLK @13.56Mhz/8
void delay_us(uint16_t delay){
	TA2CTL = 0;
	TA2CCR0 = delay;
    TA2CCTL0 = CCIE;
	TA2CTL = TASSEL_2 | ID_3 | MC__UP;					//SMCLK, 1.695Mhz
	__bis_SR_register(LPM4_bits | GIE);
}

//lowPower sleep for ms level sleep
void lowPowerSleep (uint16_t	duration) {
	//isDoingLowPwrSleep = TRUE;
	//_BIC_SR(OSCOFF);									//Enable ACLK
	TA2CCR0 = duration;
	TA2CTL = 0;
	TA2CCTL0 = CCIE;
	TA2CTL  = TASSEL__ACLK + MC__UP + TACLR;			//ACLK, 32Khz,
	__bis_SR_register(LPM3_bits | GIE);					//LPM3 sleep
	//_BIS_SR(OSCOFF); // Disable ACLK
	//isDoingLowPwrSleep = FALSE;
	return;
}

//low power Sleep for above 10ms sleep
void long_lowPowerSleep (uint16_t	duration) {
	//_BIC_SR(OSCOFF);									//Enable ACLK
	TA2CCR0 = duration;
	TA2CTL = 0;
	TA2CCTL0 = CCIE;
	TA2CTL  = TASSEL__ACLK | MC__UP | TACLR | ID_3;		//ACLK, 4k=32K/8hz,
	__bis_SR_register(LPM3_bits | GIE);					//LPM3 sleep

	return;
}
/*******************************************************************************
 @brief	LPM3 sleep or NFC timeout ISR for NFC Rx  using SMCLK or ACLK
 	 @note	only used when MCLKREQEN=1
*******************************************************************************/
#pragma vector=TIMER0_A0_VECTOR
__interrupt void timeTrackingISR(void) {
	//it is in doRFID_14443B routine
	if(doNFC_state<NFC_Tx){
			TA0R = 0;
			RX_IFG &= ~RX_BIT;
			__bic_SR_register_on_exit(LPM4_bits | GIE);
	//it is in sleep_untill_edge timeout routine
	}else if(doNFC_state&NFC_Start){
			doNFC_state = NFC_Sleep;
			__bic_SR_register_on_exit(LPM4_bits | GIE);
	}
}


/**
 * Real time CLK routine for wake up after designed time
 */
#pragma vector=TIMER1_A0_VECTOR
__interrupt void timeOutISR(void) {
	if(RTC_ctr==0xFFFF)RTC_ctr=0; //avoid RTC_ctr overflow
	RTC_ctr++;
	if(RTC_ctr>=senseDelay){
		senseState++;
		RTC_ctr=0;
		if((doNFC_state&NFC_LPM4)){
			RX_WAKEUP_IE &= ~RX_WAKEUP_BIT;
			RX_WAKEUP_EN_DIR &= ~RX_WAKEUP_EN_BIT;
			RX_WAKEUP_IFG &= ~(RX_WAKEUP_BIT);
			doNFC_state = NFC_Sleep;
			imageUpdateState = SENSE_UPDATE;
			RTC_ctr = 0;
			__bic_SR_register_on_exit(LPM4_bits | GIE);
		}

	}
}


/*******************************************************************************/
/** @fcn	lowPowerSleep ISR for TimerA2
 *  @brief	LPM3 sleep for us~ms level
 *  @note	only used when ACLKREQEN=1
 */
/*******************************************************************************/
#pragma vector=TIMER2_A0_VECTOR
__interrupt void LPM3_TimerISR(void) {
	TA2CTL = 0x00;
	TA2CCTL0 = 0x00;
  __bic_SR_register_on_exit(LPM3_bits | GIE);
}
