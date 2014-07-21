/******************************************************************************
 * 14443_B.c
 *
 * @date May 31, 2013
 * @author Derek Thrasher (UMass Amherst)
 ******************************************************************************/

#include "_14443_B.h"

volatile uint16_t i;

void initialize_14443(void)
{
  //TI_SetDCO(TI_DCO_2MHZ);                   // set DCO to 1 MHz according to DCO_Library.h
	UCA1CTL1 |= UCSWRST; //reset state alows us to reconfigure USART

	RX_UART_DIR &= ~RX_UART_BIT; //set 4.5 as input pin
	RX_UART_SEL |= RX_UART_BIT;                            // P4.5 = UCA1RXD
	_BIS_SR(OSCOFF);                          // XTAL not used

	UCA1CTL0 = 0x00; //8-bit
	UCA1CTL1 |= UCSSEL_2; //smclk
#ifdef _6_78MHz
	//N = 6.78MHz / 115.2khz = 58.85416
	//Over-sampling (UCOS16) BR0 = 58.85416/16 = 3
	//modulation as told by table 37-4 UCBRSx Settings for Fractional Portion of N
	UCA1BR0 = 3;
	UCA1BR1 = 0x00;
	UCA1MCTL |= UCBRF_10 | UCBRS_7 | UCOS16;
#else //13.56MHz
	//N = 13.56MHz / 115.2khz = 117.7083
	//Over-sampling (UCOS16) BR0 = 117.7083/16 = 7.3567
	//modulation as told by table 37-4 UCBRSx Settings for Fractional Portion of N
	UCA1BR0 = 7;
	UCA1BR1 = 0x00;
	//TODO: UCBRS_x need to be changed to match .7083(place breakpoint in 14443_protocol.c condition 0x05)
	//UCBRF = INT((7.3567-7)*16) = 5
	UCA1MCTL |= UCBRF_5 | UCBRS_5 | UCOS16;
#endif
	UCA1ABCTL = 0x00; //AUTO-BAUD disabled

	UCA1CTL1 &= ~UCSWRST; 		// Initalize USART state machine
	UCA1IE |= UCRXIE;                   // Enabled USART0 RX interrupt

	_BIS_SR(/*LPM0_bits +*/ GIE);                 // Enter LPM0 w/ interrupt


	TA0CTL |= 	TASSEL_2 |	//SMCLK as source
				MC_2 |		//CONTINUOUS MODE
				ID_3; 		//DIVIDE BY 8
	for(int16_t i = 0; i < CMD_BUF_SIZE; i++)
		rx_buffer[i] = 0;
}

unsigned short find_delimiter(void){

	//----//
	// TODO Are we intending to clear particular other bits in RX_IE and RX_IES?
	// !!!-Comm's doesn't work when we DON'T clear the other bits... why not?
	RX_IE = RX_BIT;
	RX_IES = RX_BIT;
	//----//

	UCA1IE |= UCRXIE;                   // Enabled USART0 RX interrupt

	while((TA0R < TIME_OUT) && (_14443_delimiter_found != 2))
	{

		if(_14443_delimiter_found == 1)
		{
			RX_IE &= ~RX_BIT; //disable port1.x interrupt
			_14443_delimiter_found = 0;
			return 1;
		}
		else
			continue;
	}
	return 0;
}


