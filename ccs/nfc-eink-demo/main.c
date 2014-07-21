/******************************************************************************
 *	@author Jeremy Gummeson, Derek Thrasher, Aaron Parks,
 *	Chris Marquardt, Kendall Lowrey
 *
 *  @date -	08/29/2013
 *
 * @todo Recommend moving any protocol-specific stuff out of main.c for better
 *       encapsulation of functionality
 *
 * @todo Recommend placing all state information into a single struct for easier
 *       tracking, passing, and initialization
 *
 * @todo Recommend getting rid of #ifdefs where possible and instead #including
 *       different modules to enable different protocols and functionality.
 *
 *
 * @todo Power management!
 *
 * @todo Figure out why it takes forever for reader to discover our tag, even
 *       while the subsequent transactions are mostly successful.
 *
 * @todo Would be good to implement a watchdog routine since this is supposed to
 *       be a long term battery-operated device and has no manual reset switch.
 *
 * @todo Move all protocol/driver modules into a folder and link to that instead
 *       of individually referencing each file.
 *
 * @todo Ultimately we want to be able to handle both 15693 and 14443. Develop
 *       a strategy for making that happen?
 *
 * @todo We should move all comms and driver stuff out to a project which
 *       compiles as a static library instead of individually linking source files.
 *
 *****************************************************************************/
//=============================================================================
//									Includes
//=============================================================================
// Uncomment the following line for 15693 1-out-of-256 protocol mode. Otherwise
//  we will use ISO 14443B
//#define _15693_1of256
#include "../common/globals.h"

// TODO Get rid of ifdefs if possible
#ifdef _15693_1of256
#include "../common/1outof256.h"
#include "../common/send_hdr.h"
#include "../common/15693_Protocol.h"
#else
#include "../common/_14443_B.h"
#include "../common/_14443_B_protocol.h"
#include "../common/NFC_WISP_Protocol.h"
#include "../common/send_bpsk.h"
#include "../common/crc_checker.h"
#endif

#include "../common/e-paper.h"
#include "../common/fram_memory.h"
#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

//=============================================================================
//									NFC Globals
//=============================================================================

//This stuff is totally global in nature. I'm not sure it belongs here. But here it is.
//After the CRC is verified, this gets set to the number of bytes we received.
uint8_t num_frame_bytes;

// TODO Get rid of ifdefs if possible
#ifdef _15693_1of256

extern uint8_t transmitCommand[CMD_BUF_SIZE];

unsigned char uid[UID_SIZE]; /**< Byte array that stores the UID. Initialized by _15693_protocol.c. */
unsigned int numReceivedMessages; /**< Number of messages received from a reader. Used to track image transfer progress. */
unsigned char pending_captures; /**< Tracks pending captures for ISO_15693. This is the number of outstanding pulses to be processed. */
extern unsigned int numberOfBitsTX; /**< Number of bits to transmit. Transmit function uses this to know how much of the buffer to send. */
//unsigned int bit_count; //used which checking crc

//externs
extern unsigned int capture_buffer[CAPTURE_BUFFER_SIZE];
extern unsigned char capture_buffer_write_index;

unsigned char receiveCommand[RECEIVE_BUF_SIZE];//received command
#else // otherwise assume 14443_B
unsigned char _14443_buf_ptr = 0; /**< 14443_rx buffer pointer. Points to the current buffer index of received buffer. */
unsigned char rx_buffer[CMD_BUF_SIZE]; /**< 14443_B RX message buffer. Stores CMD_BUF_SIZE bytes. */
unsigned char transmitCommand[CMD_BUF_SIZE]; /**< 14443_B TX message buffer. Stores CMD_BUF_SIZE bytes. */
unsigned char uid[UID_SIZE]; /**< Byte array that stores the PUPI. Initialized by _14443_B_protocol.c. */
unsigned short _14443_delimiter_found = 0; /**< 14443 B Delimiter Status. Indicates whether or not a valid delimiter has been found. */
extern unsigned int numberOfBitsTX;	/**< Number of bits to transmit. Transmit function uses this to know how much of the buffer to send. */
#endif /* _15693_1of256 */

unsigned long timeout_ctr;

//=============================================================================
//									E-paper Globals
//=============================================================================
unsigned char imageBuffer[E_INK_SIZE]; //big array that stores all the display bits
unsigned char imageTransferDoneFlag;
EPD_type cog; // Identifies the chip-on-glass e-ink driver we are interfacing with

//=============================================================================
//									Assorted Globals
//=============================================================================
unsigned char led_pulse_flags = 0; // Flags indicating which LED to pulse in the timer ISR

//=============================================================================
//									NFC Defines
//=============================================================================
//6.78Mhz where 1 etu is clock periods
// TODO Get rid of ifdefs if possible
#ifdef _14443_B
#ifdef _6_78MHz // 6.78 MHz clock frequency

#define TEN_PULSES 70//80
#define ELEVEN_PULSES 98//88
#define EOF 86

#else // Otherwise assume 13.56Mhz clock frequency

#define TEN_PULSES 156//160
#define ELEVEN_PULSES 180//176
#define EOF 180

#endif /* _6_78MHz */
#endif /* _14443_B */

#define TIMEOUT_COUNTS 100
//=============================================================================
//								Function definitions
//=============================================================================

/**
 * Reset the timeout counter
 */
void timeout_clear(void) {
	timeout_ctr = 0;
}

/**
 * Check to see if communications have timed out, and if it's OK to sleep
 *
 */
unsigned int timeout_occurred(void) {

	// If we still have an LED pulse to complete, don't report timeout.
	return (timeout_ctr > TIMEOUT_COUNTS) && !(led_pulse_flags);
}

/**
 * Configure and start the timeout tracking system
 */
void timeout_init(void) {
	// Initialize timeout counter
	timeout_clear();

	// Set up TA1.0 for periodic interrupt
	TA1CCR0 = 328; // 10 ms with 32768 Hz ACLK

	// Configure and start TA1
	TA1CTL = TASSEL_1 | ID_0 | MC_1 | TACLR;

	TA1CCTL0 |= CCIE; // Set up interrupt on TA1.0. ISR keeps track of time.

}

/**
 * Halt the timeout system (e.g., to save power)
 */
void timeout_halt(void) {
	TA1CTL = 0x00;
	TA1CCTL0 = 0;
}

/**
 * Initialize the E-paper subsystem.
 *
 * @todo Move this routine into epaper.c/h module and get rid of extra layers
 */
void initEPD(void) {
	SPI_initialize();
}

/**
 * Initialize the FRAM module and put it to sleep
 *
 * @todo Move this module somewhere more appropriate
 */
void initFRAM(void) {

	// Put FRAM to sleep
	SPI_FRAM_Enter_Sleep();
}

/**
 * Initialize the RFID subsystem
 *
 * @todo Move this routine into new nfc.c/h module and get rid of extra layers
 */
void initRFID(void) {
// TODO Get rid of ifdefs if possible
#ifdef _15693_1of256

	initialize_1outof256(); //sets up globals for 1 out of 256 decoding.
	initialize_15693_protocol();
	initialize_hdr();//sets up TimerB / P4.7 for PWM-based high data rate transmission

#else

	initialize_14443();				// Sets up globals and UART RX
	initialize_14443_B_protocol(); // Sets up globals for selected protocol decoding
	initialize_nfc_wisp_protocol();
	initialize_bpsk();
	imageTransferDoneFlag = 0;		// Clear flag
#endif
}


/**
 * Go into lowest possible power state until data edges are detected from the
 *  wakeup comparator
 *
 * @note It's important that all the MODxxxREQEN bits are low in UCSCTL6
 */
void sleep_until_edges(void) {

	// TODO Turn off all non-essential systems

	// RX comparator off
	RX_ENABLE_DIR &= ~RX_ENABLE_BIT;
	RX_ENABLE_OUT &= ~RX_ENABLE_BIT;
	////

	// Switch to LFXT while we turn off the XT2
	UCSCTL4 = SELS_0 | SELM_0 | SELA_0;		// SMCLK=LFXT, MCLK=LFXT

	// HF crystal off
	UCSCTL6 |= XT2OFF;

	// Setup interrupt on edges

	RX_WAKEUP_IES |= RX_WAKEUP_BIT; 	// Falling edge wakeup
	RX_WAKEUP_IFG &= ~(RX_WAKEUP_BIT);	// Clear flag prior to enabling interrupt
	RX_WAKEUP_IE |= RX_WAKEUP_BIT; // Enable interrupt

	// TODO HACK ALERT Workaround for bus problem
	SPI_MOSI_SEL &= ~(SPI_CLK_BIT | SPI_MOSI_BIT | SPI_MISO_BIT);

	// Go to sleep until edges occur
	__bis_SR_register(LPM4_bits  | GIE);

	asm(" NOP");

	// TODO HACK ALERT Workaround for bus problem
	SPI_MOSI_SEL |= (SPI_CLK_BIT | SPI_MOSI_BIT | SPI_MISO_BIT);

	// Disable interrupt
	RX_WAKEUP_IE &= ~RX_WAKEUP_BIT;

	// RX comparator on
	RX_ENABLE_DIR |= RX_ENABLE_BIT;
	RX_ENABLE_OUT |= RX_ENABLE_BIT;
	////

	UCSCTL6 &= ~XT2OFF; // Make sure XT2 is ON
	UCSCTL3 |= SELREF_2; // FLLref = REFO

	// Loop until XT1,XT2 & DCO stabilizes - in this case loop until XT2 settles
	do
	{
		UCSCTL7 &= ~(XT2OFFG | XT1LFOFFG | DCOFFG); // Clear XT2,XT1,DCO fault flags
		SFRIFG1 &= ~OFIFG;						// Clear fault flags
	}while (SFRIFG1&OFIFG);					// Test oscillator fault flag

	UCSCTL6 &= ~XT2DRIVE1;					// Decrease XT2 Drive according to
	// expected frequency

	UCSCTL4 = SELS_5 | SELM_5 | SELA_0;		// SMCLK=MCLK=XT2

	UCSCTL7 = 0; //clear fault flags

}





/**
 * Perform one RFID transaction with reader, using ISO 15693 protocol
 *
 * @todo Move this routine into a module and clearly define interface to it
 */
void doRFID_15693(void) {
#ifdef _15693_1of256

	while(pending_captures > 0)
	{
		process_bits();
	}

	if(num_frame_bytes > 0)
	{
		_bic_SR_register(GIE); //lock to protect send

		_15693_protocol();

		if(numberOfBitsTX > 0) {
			send();
		}

		num_frame_bytes = 0;

		_bis_SR_register(GIE); //unlock
	}
#endif
}

/**
 * Perform one RFID transaction with reader, using ISO 14443-4 Type B protocol
 *
 * @todo Move this routine into a module and clearly define interface to it
 */
void doRFID_14443B(void) {

	if (find_delimiter() == 1) {

		UCA1IE |= UCRXIE;		// Enabled USART0 RX interrupt
		TA0R = 0;
		_14443_buf_ptr = 0;
		while (_14443_buf_ptr < CMD_BUF_SIZE && TA0R < EOF);

		UCA1IE &= ~UCRXIE;                   // Disable USART0 RX interrupt
		TA0R = 1; //reinitialized to calculate TR0 timing of no modulation

		if (hw_crc_validate(&rx_buffer[0], _14443_buf_ptr)) {

			_14443_B_protocol(_14443_buf_ptr);

			if (numberOfBitsTX > 16 /* More than 2 bytes*/) {
				timeout_clear(); // We're talking, so reset the LPM timeout timer
				send_bpsk();

				asm(" NOP"); // DEBUG; CRC passed, attempted to talk back
			}
		} else {
			asm(" NOP"); // DEBUG; failed CRC
		}

		_14443_buf_ptr = 0;
	}

}

/**
 * Entry point16_t for application.
 */
int16_t main(void) {

	// Initialize MSP430 and peripherals
	/// TODO Consolidate init routines in here?
	system_initialize();

	// Initialize the RFID subsystem
	initRFID();

	// Initialize the EPD subsystem
	initEPD();

	// Initialize the FRAM memory module
	initFRAM();

	// Initialize the timeout system
	timeout_init();

	// Main execution loop
	while (1) {

		// Go through one RFID transaction
		// TODO Get rid of ifdefs if possible
#ifdef _15693_1of256
		doRFID_15693();
#else
		doRFID_14443B();
#endif

		if (imageTransferDoneFlag) {

			// Update the e-paper display
			updateDisplay(imageBuffer, &cog);

			// TODO Reset state more elegantly here
			initRFID(); // Also clears imageTransferDoneFlag

		} else if(timeout_occurred()) {

			timeout_halt();
			// Go into low power mode here. Exit when we are awakened by comparator edges.
			// TODO we could stop the timeout module here, but it also handles LED flashes..?
			sleep_until_edges();

			timeout_init();

		}
	}
}

/**
 * ISR, used in 14443-B mode to handle incoming data from UART module.
 *
 * @todo Move this into _14443_B_protocol.c
 */
#pragma vector=USCI_A1_VECTOR
__interrupt void usart0_rx(void) {
#ifdef _14443_B
	if (UCA1IFG & UCRXIFG) {
		//makes sure you don't load values outside of Max Buffer size
		if (_14443_buf_ptr < CMD_BUF_SIZE) {
			rx_buffer[_14443_buf_ptr++] = UCA1RXBUF;
			TA0R = 0;
			//toggle_debug();
		}
		UCA1IFG &= ~UCRXIFG;
	}
#endif
	return;
}

/**
 * Timer A0 CCR1 ISR
 * This ISR needs to be short and sweet.  When did the capture occur? Keep track of a set of captures.
 * In main, find the SOF. After that, find the bytes. After that, the EOF. Process the command. Send a response. Nice and simple.
 *
 * @todo Move this into new _15693.c/h module
 */
#pragma vector=TIMER0_A1_VECTOR
__interrupt void timerISR(void)				//6
{
// TODO Get rid of ifdefs if possible
#ifdef _15693_1of256
	if (TA0IV & TA0IV_TA0CCR2) {
		TA1R = 0x0000;//clear the timeout timer, so we don't timeout during potential frame reception

		if (pending_captures < CAPTURE_BUFFER_SIZE) {
			capture_buffer[capture_buffer_write_index] = TA0CCR2;
			if (capture_buffer_write_index == (CAPTURE_BUFFER_SIZE - 1))
			capture_buffer_write_index = 0;
			else
			capture_buffer_write_index++;

			pending_captures++;
		}

		TA0CCTL2 &= ~CCIFG;
		LPM0_EXIT;
		return;
	}
#endif
	return;
}

/**
 * Timer A1 CCR0 Interrupt Vector.
 * _15693 mode: This is where we timeout and go back to sleep.
 *
 */
#pragma vector=TIMER1_A1_VECTOR
__interrupt void timeoutISR(void)				//6
{
// TODO Get rid of ifdefs if possible
#ifdef _15693_1of256
	//check for interrupt cause
	if (TA1IV & TA1IV_TA1CCR1) {
		TA1CTL &= ~MC_0; 			//stop the timer
		TA1R = 0x0000;//clear the timer
		//RX_EN_OUT &= ~RX_EN_PIN; 			//shut off bit capture comparator
		RX_WAKEUP_EN_OUT |= RX_WAKEUP_EN_BIT;//turn on wakeup comparator
		TA0CCTL1 &= ~CCIFG; 	//clear interrupt flags
		TA1CTL &= ~TAIFG;
		_bis_SR_register(LPM4_bits + GIE);//deep sleep
	}
#endif
	return;
}


/**
 * Keeps track of time so we can implement timeout mechanism, etc.
 * Also handles automatic LED pulse generation
 *
 * @todo Move this to globals.c or some such place
 */
#pragma vector=TIMER1_A0_VECTOR
__interrupt void timeTrackingISR(void) {

	timeout_ctr++;

	/*
	 * Pulse the LEDs if directed by the application
	 */
	if(led_pulse_flags & KILL_LEDS_FLAG) {
		// If we noted that LEDs were left on, turn them off
		led_1_off();
		led_2_off();
		led_pulse_flags &= ~KILL_LEDS_FLAG;
	}

	if(led_pulse_flags & LED_1_PULSE_FLAG) {
		// LED 1 - Start pulse
		led_1_on();
		led_pulse_flags &= ~LED_1_PULSE_FLAG;
		led_pulse_flags |= KILL_LEDS_FLAG;
	}

	if(led_pulse_flags & LED_2_BIT_PULSE_FLAG) {
		// LED 2 - Start pulse
		led_2_on();
		led_pulse_flags &= ~LED_2_BIT_PULSE_FLAG;
		led_pulse_flags |= KILL_LEDS_FLAG;
	}
}

/**
 * triggered on initial NFC activity. Exit Deep sleep.
 * 1. Turn on Rx comparator
 * 2. Start a long timeout.
 * 3. Clear PORT1 interrupt
 *
 * @todo Move this into new nfc.c/h module (common to all protocols, right?)
 */
#pragma vector=PORT1_VECTOR
__interrupt void port1ISR(void) {
// TODO Get rid of ifdefs if possible
#ifdef _15693_1of256
	if (RX_WAKEUP_IFG & RX_WAKEUP_BIT) {
		RX_WAKEUP_EN_OUT &= ~RX_WAKEUP_EN_BIT; //turn off wakeup comparator
		RX_ENABLE_OUT |= RX_ENABLE_BIT;//enable bit capture comparator
		TA1CTL |= MC_1;//up mode, counts up to CCR0

		RX_WAKEUP_IFG &= ~RX_WAKEUP_BIT;//clear the interrupt
		LPM4_EXIT;//exit sleep, get ready to handle pulses.
	}
#else
	// Capture timer value ASAP after entry
	// TODO do this in timer hardware instead
	unsigned int ta0r_tmp = TA0R;

	//adding find delimiter code for p1.0 (Derek Thrasher)
	//RX_BIT is changed from 1.0 -> 1.3 in version 0.3
	if (RX_IFG & RX_BIT) {
		//falling edge
		if (RX_IES == RX_BIT) {
			TA0R = 0;
			RX_IES &= ~RX_BIT;
			_14443_delimiter_found = 0;
			//TA0CTL = TASSEL_2 + MC_2 + ID_3;
		}

		//rising edge
		else {

			if ((ta0r_tmp > TEN_PULSES) && (ta0r_tmp < ELEVEN_PULSES)
					&& (_14443_delimiter_found == 0)) {
				_14443_delimiter_found = 1;
			} else
				_14443_delimiter_found = 2;

		}
		RX_IFG &= ~RX_BIT;
	}

	if (RX_WAKEUP_IFG & RX_WAKEUP_BIT) {
		// Clear interrupt flag
		RX_WAKEUP_IFG &= ~RX_WAKEUP_BIT;

		// Exit low power mode on RETI
		__bic_SR_register_on_exit(LPM4_bits);
	}

#endif
	return;
}

