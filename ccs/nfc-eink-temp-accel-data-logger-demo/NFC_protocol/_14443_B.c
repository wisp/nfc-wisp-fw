/******************************************************************************
 * 14443_B.c
 *
 * @date May 31, 2013
 * @author Derek Thrasher (UMass Amherst)
 ******************************************************************************/

#include "_14443_B.h"

uint8_t validRx=0;

//************************************************************************************************************
/* @func	if detect delimiter within Five ETU(5*9.3us) then decode data otherwise wake up
 * @note	doNFC_state=1 is set in sleep_untill_read
 *************************************************************************************************************/
unsigned short doNFC_Rx(void){
	//----//
	// TODO Are we intending to clear particular other bits in RX_IE and RX_IES?
	// !!!-Comm's doesn't work when we DON'T clear the other bits... why not?
	// RX comparator on
	Enable_Rx();
	//toggle_led_2();
	validRx = 0;//reset in RX_WAKEUP ISR
	delay_us(T_35us);				//let RX comparator stable, because sometimes there is a noise when enable rx comparator
	doNFC_state = NFC_Rx0;			//Flag is Rx,used for measure the EOF
	RX_IE |= RX_BIT;
	//toggle_led_1();
	/*test delimiter, if find set the timeout and wait for rx data*/
	TA0CCR0 = FIVE_PULSES;//timeout should larger than 22.2us
	TA0CCTL0 = CCIE;
	TA0CTL = TASSEL_2 + MC_2+ ID_3 + TACLR;	// time the delimiter length and then used for send_bpsk
	__bis_SR_register(LPM3_bits | GIE);
	TA0CTL = 0;
	Disable_Rx();
	TA0CCTL0 = 0;				//Disable CCIE and TA0CCR0

	//toggle_led_1();
	//doNFC_state = 0;
	return validRx;
}


