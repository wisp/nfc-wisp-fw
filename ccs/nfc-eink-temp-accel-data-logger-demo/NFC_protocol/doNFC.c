/******************************************************************************
 * doNFC.c
 *	@fun:		High level the NFC reading event handler
 *  @date: 		Aug 22, 2014
 *  @author: 	Yi Zhao (Eve)-Sensor System Lab, UW and Disney Research-Pitssburgh
 *  @note:		Only test ISO 14443-B protocol here
 *  @TODO:		Test and Implement _15693_1of256 protocol
  *********************************************************************************/


//=============================================================================
//									Includes
//=============================================================================
#include "doNFC.h"
#include "send_bpsk.h"
#include "string.h"
#include "crc_checker.h"
#include "../UserApp/myNFC_Protocol.h"
//=============================================================================
//									NFC Globals
//=============================================================================
uint8_t num_frame_bytes;
uint8_t	volatile 	doNFC_state;


#ifdef _15693_1of256

//extern uint8_t transmitCommand[CMD_BUF_SIZE];
/**< Byte array that stores the UID. Initialized by _15693_protocol.c. */
uint8_t uid[UID_SIZE];
/**< Number of messages received from a reader. Used to track image transfer progress. */
unsigned int numReceivedMessages;
/**< Tracks pending captures for ISO_15693. This is the number of outstanding pulses to be processed. */
uint8_t pending_captures;
/**< Number of bits to transmit. Transmit function uses this to know how much of the buffer to send. */
extern unsigned int numberOfBitsTX;
//unsigned int bit_count; //used which checking crc

//externs
extern unsigned int capture_buffer[CAPTURE_BUFFER_SIZE];
extern uint8_t capture_buffer_write_index;

uint8_t receiveCommand[RECEIVE_BUF_SIZE];//received command
#else // otherwise assume 14443_B

/**< 14443_rx buffer pointer. Points to the current buffer index of received buffer. */
uint8_t _14443_buf_ptr = 0;

/**< 14443_B RX message buffer. Stores CMD_BUF_SIZE bytes. */
uint8_t rx_buffer[CMD_BUF_SIZE];
/**< 14443_B TX message buffer. Stores CMD_BUF_SIZE bytes. */
uint8_t transmitCommand[CMD_BUF_SIZE];
/**< Byte array that stores the PUPI. Initialized by _14443_B_protocol.c. */
uint8_t uid[UID_SIZE];
uint8_t currentByte;
uint8_t currentBit;

uint8_t debugIndex;
//uint8_t app[APP_SIZE];

/**< Number of bits to transmit. Transmit function uses this to know how much of the buffer to send. */
//extern unsigned int numberOfBitsTX;
#endif /*use either ISO-14443 B or  _15693_1of256 */

//=======================================================================================
//									Functions
//=======================================================================================

/**
 * @func	Initialize the RFID subsystem
 *
 * @todo 	Move this routine into new nfc.c/h module and get rid of extra layers
 */
void initRFID(void) {
// TODO Get rid of ifdefs if possible
#ifdef _15693_1of256

	initialize_1outof256(); //sets up globals for 1 out of 256 decoding.
	initialize_15693_protocol();
	initialize_hdr();//sets up TimerB / P4.7 for PWM-based high data rate transmission

#else
	initialize_14443_B_protocol();  // Sets up globals for selected protocol decoding
	initialize_nfc_wisp_protocol(); //Initialize some variable
	initialize_bpsk();				// Sets up Tx setting, TimerB is not start
#endif
}


/**
 * @func	Perform one RFID transaction with reader, using ISO 15693 protocol
 *
 * @todo 	Move this routine into a module and clearly define interface to it
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
 * @func	Go to LPM4 mode wait for RX_WAKEUP, wakeup must finished within 90us
 * @note	ACLK,FLL is disabled in _system_pre_init, UCSCTL8 used to clear SMCLKREQEN,
 * 			MCLKREQEN,ACLKREQEN if do not want in LPM
 * @TODO	After RX_WAKEUP it takes less than 100us() to enable the RX
 * @note	After enable RX_WAKE_UP for at least 30us~50us and then enable interrupt,
 * 			otherwise there is some err
 */
void sleep_until_edges(void) {
	 __disable_interrupt();
	 enable_Rx_Wakeup();

	 /*Turn on timeout clock and
	  *go into deep sleep mode (turn off 13MHz crystal) if no event*/
	 timeout_init(Sleep_TimeOut);

	// UCSCTL8 &= ~(SMCLKREQEN|MCLKREQEN);			//disable SMCLK if other perfiferal use it
	 __bis_SR_register(LPM3_bits | GIE);			// Go to sleep until edges occur

	 disable_Rx_Wakeup();
	 timeout_halt();
	 //UCSCTL8 |= SMCLKREQEN|MCLKREQEN|ACLKREQEN;   //uncomment it out this if needed
}

/**
 * @fun		Perform one RFID transaction with reader, using ISO 14443-4 Type B protocol
 * @note 	If timeout without receiving any NFC signal then go to sleep_untill_read
 */
void doRFID_14443B(void) {

	uint8_t ACLK_off = 0;  //Use to fix a bug,see the warning below

	//Use lowPower Comparator to detect the SOF(0) edges and wakeup Rx
	sleep_until_edges();
	if(doNFC_state&NFC_Start){

		#warning "MSP430 BUG: must turn ACLK on here, otherwise the  \
				backscatter won't work, it may because higher power \
				can help detuing the antenna to fit for 847kHz bandwidth"

		if(!(TA1CTL&0x30)){
			TA1CCTL0 &= CCIE;
			TA1CCR0 = 0xFF;
			ACLK_off = 1;
			TA1CTL |= MC_2 + TASSEL_1 + ID_3 + TACLR;
		}

		_14443_buf_ptr = 0;

		//Whether the start of fram is correct
		if (doNFC_Rx()) {
			//If data pass crc checking
			if (hw_crc_validate(&rx_buffer[0], _14443_buf_ptr)) {
				//toggle_led_2(); //Indicate valid reading
				//Response to the NFC command
				if(_14443_B_protocol(_14443_buf_ptr)){
					//led_1_on();
					doNFC_state = NFC_Tx;
					send_bpsk();
				}
			}
			//Comment below to save power
			//memset(rx_buffer,0x00,_14443_buf_ptr); //Clean rx_buffer after process
		}

		if(ACLK_off)TA1CTL = 0;
		doNFC_state = NFC_Start;
		//led_1_off();
		//led_2_off();
	}

}

/**
 * @func		Go to LPM4 mode wait for reader to read the tag
 * @note		ACLK,FLL is disabled in _system_pre_init, UCSCTL8 used to clear SMCLKREQEN,
 * 				MCLKREQEN,ACLKREQEN if do not want in LPM
 * @TODO		after RX_WAKEUP it go to sleep_until_edge to waiting for command
 * @Note	    it may take 400us for XT2 to stablize
 */
void sleep_until_read(void) {

	    __disable_interrupt();
		//UCSCTL8 &= ~(SMCLKREQEN|MCLKREQEN);						//*disable SMCLK,ACLK to enable LPM4,
	    															//if ACLK used by User app then keep it on*/
		// Switch to LFXT while we turn off the XT2
		UCSCTL4 |= SELS_0 | SELM_0 | SELA_0;						// MCLK=LFXT
		UCSCTL6 |= XT2OFF;											// HF crystal off


		//#ifdef TempDemo
		doNFC_state=NFC_LPM4;

#warning "Must predefine HAS_SENSOR_ISR for your application, if you have other ISR than NFC ISR"

#ifdef  HAS_SENSOR_ISR
		enable_sensor_ISR();
#endif
		//#endif
		enable_Rx_Wakeup();
		__bis_SR_register(LPM4_bits| GIE);							//Go to sleep until edges occur

		//NFC state should be set by different interrupt before wakeup
		disable_Rx_Wakeup();
#ifdef HAS_SENSOR_ISR
		disable_sensor_ISR();
#endif

		UCSCTL4 |= SELS_3 | SELM_3 | SELA_2;						// SMCLK=MCLK=XT2, ACLK=XT1
		UCSCTL6 &= ~(XT2OFF); 										// Make sure XT2 is Off
		// Loop until XT2 & DCO stabilizes - in this case loop until XT2 settles
		do
		{
			UCSCTL7 &= ~(XT2OFFG | DCOFFG); 						// Clear XT2 fault flags
			SFRIFG1 &= ~OFIFG;										// Clear fault flags
		}while (SFRIFG1&OFIFG);										// Test oscillator fault flag
		UCSCTL6 &= ~XT2DRIVE1;										// Decrease XT2 Drive according to expected frequency
		UCSCTL4 |= SELS_5 + SELM_5 + SELA_0;						// SMCLK=MCLK=XT2, ACLK=XT1, must use "|="
		UCSCTL7 = 0; 												//clear fault flags
		//UCSCTL8 |= SMCLKREQEN+MCLKREQEN+ACLKREQEN; 				//Note@eve: Timer could not be used during this \
																	  sleep routine, to modify this if needed
}

/**
 * Exntern High level interface for NFC reading cycles;
 */
void doNFC(void){
	if(doNFC_state&NFC_Start){
		doRFID_14443B();		  //wait NFC reading}
	}else if(doNFC_state&NFC_Sleep){

		sleep_until_read(); //Deep sleep if no NFC reading signal
	}
}

/**
 * Timer A0 CCR1 ISR
 * This ISR needs to be short and sweet.  When did the capture occur? Keep track of a set of captures.
 * In main, find the SOF. After that, find the bytes. After that, the EOF. Process the command. Send a response. Nice and simple.
 *
 * @todo Move this into new _15693.c/h module
 */
#ifdef _15693_1of256
#pragma vector=TIMER0_A1_VECTOR
__interrupt void timerISR(void)				//6
{
// TODO Get rid of ifdefs if possible

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

	return;
}
#endif

/**
 * Timer A1 CCR0 Interrupt Vector.
 * _15693 mode: This is where we timeout and go back to sleep.
 *
 */
#ifdef _15693_1of256
#pragma vector=TIMER1_A1_VECTOR
__interrupt void timeoutISR(void)				//6
{
// TODO Get rid of ifdefs if possible

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

	return;
}
#endif
