/********************************************************************************
 * send_bpsk.c
 * @fun  	Send a command [start bit and stop bit inserted automatically
 * 			command is an array containing the bits to be sent
 *  		command size is the number of bits in the given command to send
 *  	   	command array: R12 (ARG_1), bit size: R13 (ARG_2)
 * @note 	must call 8 cycles prior to first bit (7 starts timer, 8 to get high on PWM)
 * @date 	Jan 5, 2015
 * @author	Eve(Yi Zhao), Sensor Systems Lab, UW
 * 		 	Derek Thrasher (UMass Amherst)
 ******************************************************************************/


//=============================================================================
//									Includes
//=============================================================================
#include "send_bpsk.h"
#include "doNFC.h"
#include <string.h>


//=============================================================================
//									Variables
//=============================================================================
uint8_t bpsk_ct;				//Used to count 8 bits = 1 byte between START and STOP bits
uint8_t next_bit_value;			//Used to hold value of next bit value while current bit is being sent via PWM
uint8_t bpsk_send_bit_count;	//allows us to stop transmission once all bits are sent ( always multiple of 8)
unsigned int numberOfBitsTX;	/**< Number of bits to transmit.\
 	 	 	 	 	 	 	 	 Transmit function uses this to know how much of the buffer to send. */
volatile unsigned int timer;

//=============================================================================
//									Functions
//=============================================================================
/**
 * @fun		Initial timer B for bpsk backscatter
 * @note 	must call 8 cycles prior to first bit (7 starts timer, 8 to get high on PWM)
 */
void initialize_bpsk(void)
{

	//configure timer output pin
#ifdef _6_78Mhz
	TB0CCR0 = 4; 			//upper bound compare
	TB0CCR1 = 0;			//lower bound compare
	TB0R = 3; 				//ensures that we will get a rising edge imediately
#else//13.56Mhz
	TB0CCR0 = 8; 			//upper bound compare
	TB0CCR2 = 0;			//lower bound compare
	TB0R = 7; 				//ensures that we will get a rising edge imediately
#endif

	//This set of register writes configures Pin4.7 output to Timer B CCR1 output
	//I based these settings on the TI sample code. It works
	PMAPPWD = 0x02D52;    	// Enable Write-access to modify port mapping registers
	PMAPCTL = PMAPRECFG;    // Allow reconfiguration during runtime

	//disables output driver
	P4MAP7 = PM_ANALOG;
	P4MAP7 = PM_TB0CCR2A;	//Need to remap 4.7 pin to PWM Output (need to double check this)

	//PMAPPWD = 0;			// Disable Write-Access to modify port mapping registers
	PMAPPWD = 0x096A5;
	TX_PWM_DIR |= TX_PWM_BIT; //output direction
	TX_PWM_SEL |= TX_PWM_BIT; //PWM mode

	transmitCommand[0] = 0;//To save power we only init one byte to declare var in compilier

	//memset(transmitCommand, 0x00, CMD_BUF_SIZE); //Comment it out to save power

	bpsk_ct = 0;
	next_bit_value = 0;
	bpsk_send_bit_count = 0;
}

void delay(unsigned int cycles)
{
	unsigned int i = 0;
	for(i = 0; i < cycles; i++)
		asm(" nop");
}

/**
 *
 */
void send_bpsk(void)
{
	/*Derek Thrasher
	 *Use below commands to change phase
	 *TB0CCTL1 = OUTMOD_3; 	// sets at TBCCR1 / resets at TBCCR0 (Logic 1)
	 *TB0CCTL1 = OUTMOD_7; 	// resets at TBCCR1 / sets at TBCCR0 (Logic 0)
	 */

	/*Derek Thrasher
	 * Timer B is used for PWM
	 * Timer A is a running clock that is used to compare to the incrementing 'timer' value when sending bpsk
	 */
	TA0CTL = TASSEL_2 + MC_2+ ID_3 + TACLR;
	//TODO: put this loop into LPM@eve
	delay_us(POST_RX_WAIT-8); // can not implement more lowPowerDelay since it affect on the RF load
	//NFCTxSleepOn(POST_RX_WAIT);
	timer = POST_RX_WAIT;
	while(TA0R < timer);	//TR0
	//TA0CTL = TASSEL_2 + MC_2+ ID_3 + TACLR;

	//ACLK_on();
	//_BIC_SR(OSCOFF);
	ENABLE_PWM();
	//Send setup
	bpsk_ct = 0;
	next_bit_value = 0;
	bpsk_send_bit_count = 0;
	//reset bit mask and bit count
	asm(" MOV	#0001h, SEND_BIT_MASK");			//1
	//Store transmitCommand adr into SEND_COMMAND
	asm(" MOV #transmitCommand, SEND_COMMAND");
	//SEND_CURRENT_WORD is the current byte being transmitted. Is updated in shift_mask() after each byte
	asm(" MOV	@SEND_COMMAND, SEND_CURRENT_WORD");	//2

	timer += TEN_ETU+2;
	//ACLK_on();
	delay_us(TEN_ETU_SLEEP);		//Shold sleep TEN_ETU-10, longger than that won't work

	while(TA0R < timer);			//TR1 = count 80 pwm pulses

	//SOF
	TB0CCTL2 =	OUTMOD_7; 			//Logic 0 for 10 ETU's (1ETU = 9.88us)
	timer += TEN_ETU;
	delay_us(TEN_ETU_SLEEP);		//Shold sleep TEN_ETU-10, sleep longger than that won't work

	while(TA0R < (timer));
	TB0CCTL2 =	OUTMOD_3; 			//Logic 1 for 2 ETU's (1ETU = 9.88us)

	timer += TWO_ETU;
	delay_us(TWO_ETU_SLEEP);
	while(TA0R < (timer));


	//Start sending message
	//Start bit is sent prior to entering the while loop
	//This method is used so that we can send the START bit immediately\
	  after the STOP bit at the end of the while loop.
	//This method does not affect the EOF since both the START bit and \
	  beginning of EOF are logic 0
	TB0CCTL2 =	OUTMOD_7; //Logic 0

	while(bpsk_send_bit_count < numberOfBitsTX)
	{


		//Derek Thrasher - Need to load next bits during previous wait period\
		  in order to meet timing constraints.
		next_bit_value = next_bit();
		//bpsk_send_bit_count = bpsk_send_bit_count + 1;

		while(bpsk_ct < 0x08)
		{
			if(next_bit_value == 1)
			{
				timer += ONE_ETU;
				while(TA0R < timer);
				//Logic 1
				TB0CCTL2 =	OUTMOD_3;

			}
			else
			{
				timer += ONE_ETU;
				while(TA0R < timer);
				//Logic 0
				TB0CCTL2 =	OUTMOD_7;

			}

			//don't itterate on last bit since we determin 1st bit,\
			  before entering while loop (1+7 = 8 = 1 byte)
			if(bpsk_ct < 0x07)
			{
				next_bit_value = next_bit();
				//bpsk_send_bit_count = bpsk_send_bit_count + 1;

			}
			bpsk_ct = bpsk_ct + 1;
		}
		bpsk_ct = 0; //reinitialize bpsk_ct for next byte

		timer +=ONE_ETU;
		while(TA0R < timer);	//finish sending last bit

		//Always a stop bit
		TB0CCTL2 =	OUTMOD_3;	//Logic 1
		timer += ONE_ETU;
		while(TA0R < timer);

		//start bit
		TB0CCTL2 =	OUTMOD_7;	//Logic 0
	}

	//ACLK_on();
	timer += TEN_ETU;
	delay_us(TEN_ETU_SLEEP);	//Shold sleep TEN_ETU-20, longger than that won't work
	while(TA0R < (timer));

	TB0CCTL2 =	OUTMOD_3; //Logic 1
	//TODO magic number +4 may not be needed

	timer += ONE_ETU;//+4;
	while(TA0R < timer);

	//Make sure TX_PWM_BIT output is LOW!
	TB0CCTL2 =	OUTMOD_0; //Logic 1
	timer += 4;//4give it a couple cycles to settle.
	while(TA0R < timer);



	DISABLE_PWM();
	TA0CTL = 0;
	TA0CCTL0 =0;

	#ifdef _6_78Mhz
	TB0CCR0 = 4; 			//upper bound compare
	TB0CCR1 = 0;			//lower bound compare
	TB0R = 3; 				//ensures that we will get a rising edge imediately
	#else//13.56Mhz
	#endif

	return;
}

/**
 *
 */
void shift_mask()
{

	//shift mask
	asm(" RLA	SEND_BIT_MASK");				//1
	//increase address/reset bit mask if needed
	asm(" CMP	#0, SEND_BIT_MASK");			//1
	asm(" JEQ	SEND_DELAY_RESET_A");			//2
	asm(" JMP	SEND_DELAY_RESET_POST_A");	//2
	//otherwise shift mask and continue
	asm("SEND_DELAY_RESET_A:");
	asm(" MOV	#0001h, SEND_BIT_MASK");	//1
	asm(" ADD	#2, SEND_COMMAND");			//1
	asm(" MOV	@SEND_COMMAND, SEND_CURRENT_WORD");	//2
	asm("SEND_DELAY_RESET_POST_A:");
	return;
}

/**
 *
 */
uint8_t next_bit()
{
	uint8_t volatile ret;
	bpsk_send_bit_count = bpsk_send_bit_count + 1;
	//asm(" INC	SEND_BIT_COUNT");					//1
	//get current bit from current word
	asm(" BIT.W SEND_BIT_MASK, SEND_CURRENT_WORD");	//1
	asm(" JNZ NEXT");
	ret = 0x00;
	asm(" JMP end_next_bit");
	asm("NEXT: ");
	ret = 0x01;
	asm("end_next_bit: ");
	shift_mask();
	return ret;
}
