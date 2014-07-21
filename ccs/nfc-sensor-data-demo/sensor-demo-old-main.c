#include <msp430.h>

int main(void) {
	return 0;
}

#ifdef THIS_IS_REALLY_OLD

/******************************************************************************
 *	@author Jeremy Gummeson, Derek Thrasher, Aaron Parks,
 *	Chris Marquardt, Kendall Lowrey
 *
 *  @date 03/11/2013
 *
 *	Clock - 6.78Mhz = 13.56/2
 *
 * @todo This is no longer compatible with common protocol/driver modules
 *****************************************************************************/
//=============================================================================
//									Includes
//=============================================================================

#include "globals.h"
#include "1outof256.h"
#include "send_hdr.h"
#include "protocol.h"
#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include "crc_checker.h"

//=============================================================================
//									Defines
//=============================================================================


//This stuff is totally global in nature. I'm not sure it belongs here. But here it is.
//After the CRC is verified, this gets set to the number of bytes we received.
uint8_t num_frame_bytes;

//Every time we get a 1-out-of-256 pulse, we store it and process after ISR is serviced. # of pulses left to process.
uint8_t pending_captures;

//transmitted command
extern uint8_t transmitCommand[CMD_BUF_SIZE];
uint8_t uid[UID_SIZE];

uint16_t imageBytesReceived;


extern uint16_t numberOfBitsTX;	//number of bits to transmit
//uint16_t bit_count; //used which checking crc

//externs
extern uint16_t capture_buffer[CAPTURE_BUFFER_SIZE];
extern uint8_t capture_buffer_write_index;

uint8_t receiveCommand[RECEIVE_BUF_SIZE];			//received command

uint8_t imageBuffer[E_INK_SIZE]; //big array that stores all the display bits

uint16_t bad_interrupts;

uint8_t txCounter;

typedef enum {
	EPD_1_44,        // 128 x 96
	EPD_2_0,         // 200 x 96
	EPD_2_7          // 264 x 176
} EPD_size;

typedef enum {           // Image pixel -> Display pixel
	EPD_compensate,  // B -> W, W -> B (Current Image)
	EPD_white,       // B -> N, W -> W (Current Image)
	EPD_inverse,     // B -> N, W -> B (New Image)
	EPD_normal       // B -> B, W -> W (New Image)
} EPD_stage;

typedef struct {
	EPD_size size ;
	uint16_t lines_per_display;
	uint16_t dots_per_line;
	uint16_t bytes_per_line;
	uint16_t bytes_per_scan;
	const uint8_t *gate_source;
	uint16_t gate_source_length;

	bool filler;
} EPD_type;



void EPD_initialise(EPD_type *cog);
void EPD_frame_fixed(EPD_type *cog, uint8_t fixed_value);
void EPD_line(EPD_type *cog, uint16_t line, const uint8_t *data, uint8_t fixed_value, EPD_stage stage);
void EPD_finalise(EPD_type *cog);
void EPD_frame_data(EPD_type *cog, const uint8_t *image, EPD_stage stage);

#define PORTJ_DEBUG     BIT0

//#define PORT4_LED	      BIT3
#define PORT4_SPI_CLK_BIT     BIT3
#define PORT4_SPI_MISO_BIT	  BIT2
#define PORT4_SPI_MOSI_BIT	  BIT1

#define PORT5_BUSY		  BIT1

//Port 6
#define PORT6_RESET		  BIT3
#define PORT6_PANEL_ON	  BIT2 //PWR_ON

//Port 1
#define PORT1_DISCHARGE	  BIT6
#define PORT1_BORDER	  BIT4
#define PORT1_EPD_CS	  BIT5
#define PORT1_PWM		  BIT2

#define CLK_MHz 16
#define SPI_BPS 10000000


//#define PORT4_DIR   (PORT4_LED)
#define PORT6_DIR	(PORT6_RESET | PORT6_PANEL_ON)
#define	PORT1_DIR	(PORT1_DISCHARGE | PORT1_BORDER | PORT1_EPD_CS )
#define PORTJ_DIR   (PORTJ_DEBUG)

#define PORT1_INIT   0
#define PORT4_INIT   0
#define PORT5_INIT   0
#define PORT6_INIT   0

void send(void);
//=============================================================================
//								Function definitions
//=============================================================================
int16_t main(void)
{ 
	//disable watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	_bic_SR_register(GIE); //Turn off GIE while we initialize.
	system_initialize();	//gpios

	//Using FLL
	UCSCTL3 = SELREF_0;							//XTL1
	UCSCTL4 |= SELA_0;							// Set ACLK = XT1
	UCSCTL0 = 0x0000;                           // Set lowest possible DCOx, MODx

	// Loop until XT1,XT2 & DCO stabilizes - In this case only DCO has to stabilize
	do
	{
		UCSCTL7 &= ~(XT2OFFG | XT1LFOFFG | DCOFFG);
		// Clear XT2,XT1,DCO fault flags
		SFRIFG1 &= ~OFIFG;                      // Clear fault flags
	}while (SFRIFG1&OFIFG);                   // Test oscillator fault flag

	__bis_SR_register(SCG0);                  // Disable the FLL control loop

	UCSCTL1 = DCORSEL_5;                      // Select DCO range 16MHz operation
	UCSCTL2 = 206;                           // Set DCO Multiplier for 6.78 MHz
	// (N + 1) * FLLRef = Fdco
	// (243 + 1) * 32768 = 8MHz
	__bic_SR_register(SCG0);                  // Enable the FLL control loop

	//timeout_initialize(TIMEOUT_VAL); //timer A0 for power timeouts

	imageBytesReceived = 0;    //this counter tells us how many of the image bytes we have received

	initialize_1outof256(); 	//sets up globals for 1 out of 256 decoding.
	initialize_protocol(); 		//sets up globals for 15693 protocol decoding
	initialize_hdr();			//sets up TimerB / P4.7 for PWM-based high data rate transmission

	bad_interrupts = 0;

	//bit_count = 0;

	//main execution loop
	while(1){


		while(pending_captures > 0)
		{
			process_bits();
		}

		if(num_frame_bytes > 0)
		{
			//toggle_debug();
			toggle_led_0();
			_bic_SR_register(GIE); //lock to protect send
			protocol();
			//toggle_debug();
			if(numberOfBitsTX > 0) {
				txCounter++; // Keep track of number of transmits
				send();
			}
			num_frame_bytes = 0;
			_bis_SR_register(GIE); //unlock
		}

		if(imageBytesReceived == E_INK_SIZE)
			break;


		/* START HACK - After done talking to reader, update data (change UID) and compute CRC */

		uid[7] = txCounter;

		uint8_t index = 0;
		uint8_t numberTxBits;
		uint8_t copy_i;

		transmitCommand[index++] = 0x00; // response_flags=0x00, no error, no protocol format extension;
		transmitCommand[index++] = 0x00; //DSFID=0x00: VICC programming not supported

		// TODO: Figure out a way where we don't need array copies if possible
		for(copy_i=0 ; copy_i<8 ; copy_i++){
			transmitCommand[index++] = uid[copy_i];
		}

		numberTxBits = index*8;

		hw_crc_ccitt(transmitCommand, numberTxBits);

		/* END HACK */

		//enable interrupts and go to sleep
		_bis_SR_register(LPM0_bits + GIE);

	} /* while(1) */

	return 0;
}


//Timer A0 CCR1 ISR
//This ISR needs to be short and sweet.  When did the capture occur? Keep track of a set of captures.
//In main, find the SOF. After that, find the bytes. After that, the EOF. Process the command. Send a response. Nice and simple.
#pragma vector=TIMER0_A1_VECTOR
__interrupt void timerISR(void)				//6
{
	if(TA0IV & TA0IV_TA0CCR2)
	{
		TA1R = 0x0000;		//clear the timeout timer, so we don't timeout during potential frame reception

		if(pending_captures < CAPTURE_BUFFER_SIZE)
		{
			capture_buffer[capture_buffer_write_index] = TA0CCR2;
			if(capture_buffer_write_index == (CAPTURE_BUFFER_SIZE -1))
				capture_buffer_write_index = 0;
			else
				capture_buffer_write_index++;

			pending_captures++;
		}

		TA0CCTL2 &= ~CCIFG;
		LPM0_EXIT;
		return;
	}
	return;
}

//Timer A1 CCR0 Interrupt Vector.

//This is where we timeout and go back to sleep.
//1. Stop Timer A1.
//2. Reset the count value so it's ready to go for next time.
//3. Disable Capture comparator, enable wakeup comparator.

#pragma vector=TIMER1_A1_VECTOR
__interrupt void timeoutISR(void)				//6
{
	//check for interrupt cause
	if(TA1IV & TA1IV_TA1CCR1)
	{
		TA1CTL &= ~MC_0; 			//stop the timer
		TA1R = 0x0000;				//clear the timer
		//PJOUT &= ~RX_EN_PIN; 			//shut off bit capture comparator
		PJOUT |= RX_WAKEUP_EN_PIN;		//turn on wakeup comparator

		TA0CCTL1 &= ~CCIFG; 	//clear interrupt flags
		TA1CTL &= ~TAIFG;
		_bis_SR_register(LPM4_bits + GIE); //deep sleep
	}
	return;
}


//triggered on initial NFC activity. Exit Deep sleep.

//1. Turn on Rx comparator
//2. Start a long timeout.
//3. Clear PORT1 interrupt
#pragma vector=PORT1_VECTOR
__interrupt void port1ISR(void)
{
	if(P1IV & P1IV_P1IFG7)
	{
		PJOUT &= ~RX_WAKEUP_EN_PIN; //turn off wakeup comparator
		PJOUT |= RX_EN_PIN; 	//enable bit capture comparator
		TA1CTL |= MC_1;			//up mode, counts up to CCR0

		P1IFG &= ~RX_WAKEUP_BIT; //clear the interrupt
		LPM4_EXIT; //exit sleep, get ready to handle pulses.
	}
	return;
}

#pragma vector=RTC_VECTOR
#pragma vector=PORT2_VECTOR
#pragma vector=TIMER2_A1_VECTOR
#pragma vector=TIMER2_A0_VECTOR
#pragma vector=USCI_B1_VECTOR
#pragma vector=USCI_A1_VECTOR
//#pragma vector=PORT1_VECTOR
//#pragma vector=TIMER1_A1_VECTOR
#pragma vector=TIMER1_A0_VECTOR
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
	// Alternatively, keep a bad interrupt counter here for debugging purposes
}

#endif
