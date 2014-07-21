/*
 * send_hdr.c
 *
 *  Created on: Mar 8, 2013
 *      Author: Jeremy Gummeson
 */

/*
   send a command [start bit and stop bit inserted automatically]
   command is an array containing the bits to be sent
   command size is the number of bits in the given command to send
   command array: R12 (ARG_1), bit size: R13 (ARG_2)
NOTE: must call 8 cycles prior to first bit (7 starts timer, 8 to get high on PWM)
 */

#include "send_hdr.h"

unsigned char transmitCommand[CMD_BUF_SIZE];
uint16_t numberOfBits;	//number of bits to transmit

void initialize_hdr(void)
{

	//configure timer output pin

	TB0CTL = TBSSEL_2;
	TB0CCTL1 =	OUTMOD_3; // sets at TBCCR1 / resets at TBCCR0 (SEE BELOW)

	TB0CCR0 = 16; 			//upper bound compare
	TB0CCR1 = 0;			//lower bound compare

	//This set of register writes configures Pin4.7 output to Timer B CCR1 output
	//I based these settings on the TI sample code. It works
	PMAPPWD = 0x02D52;    	// Enable Write-access to modify port mapping registers
	PMAPCTL = PMAPRECFG;    // Allow reconfiguration during runtime
	//disables output driver
	P4MAP7 = PM_ANALOG;
	P4MAP7 = PM_TB0CCR1A;	//Need to remap 4.7 pin to PWM Output (need to double check this)
	//PMAPPWD = 0;			// Disable Write-Access to modify port mapping registers
	PMAPPWD = 0x096A5;
	P4DIR |= PWM_PIN;		//output direction
	P4SEL |= PWM_PIN;		//PWM mode
}

void delay(uint16_t cycles)
{
	int i = 0;
	for(i = 0; i < cycles; i++)
		asm(" nop");
}

void send(void)
{
	TB0CCTL1 =	OUTMOD_3;
	//reset bit mask and bit count
	asm(" MOV	#0001h, SEND_BIT_MASK");				//1
	//move number of bits into SEND_BIT_SIZE register
	asm(" MOV &numberOfBits, SEND_BIT_SIZE");
	// Clear SEND_BIT_COUNT
	asm(" CLR SEND_BIT_COUNT");					//1
	//Store transmitCommand adr into SEND_COMMAND
	asm(" MOV #transmitCommand, SEND_COMMAND");
	//send start bit
	while(TA0R < (LONG_TX + POST_RX_WAIT)); //will get rid of magic number eventually.
	ENABLE_PWM()									//on for 225us
	TA0R = 0;

	while(TA0R < LONG_TX);
	DISABLE_PWM()
	TA0R = 0;
										//on for 75us
	while(TA0R < HALF_TX_PERIOD);
	ENABLE_PWM()
	TA0R = 0;

	asm("SEND_BEGIN:");
	//SAVE START TIME - Derek Thrasher - This will be used when CRC is implemented in between ENABLE and DISABLE of PWM
	//Will implement a dynamic delay instead of the static ones that are currently implemented.


	//check that bits remain and increment if true
	asm(" CMP	SEND_BIT_COUNT, SEND_BIT_SIZE");	//1
	asm(" JEQ	SEND_END");							//2
	asm(" INC	SEND_BIT_COUNT");					//1

	//read command
	//Derek Thrasher - Better to have this occur every time even though the data is SEND_CURRENT_WORD is only being updated every 8 iterations
	//Derek Thrasher - Keeps the timing consistent
	asm(" MOV	@SEND_COMMAND, SEND_CURRENT_WORD");	//2

	//get current bit from current word
	asm(" BIT	SEND_BIT_MASK, SEND_CURRENT_WORD");	//1
	asm(" JC	SEND_LOGIC_ONE");					//2

	//TAKES 5 CYCLES TO GET HERE FROM SEND_BEGIN:
	//logic zero
	while(TA0R < HALF_TX_PERIOD);
	ENABLE_PWM()
	TA0R = 0;
									//on for 75us
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
	asm("SEND_DELAY_RESET_POST_A:");

	//enable pwm
	while(TA0R < HALF_TX_PERIOD);
	DISABLE_PWM()
	TA0R = 0;
									//off for 75us
	//repeat
	asm(" JMP	SEND_BEGIN");					//2

	//logic one
	asm("SEND_LOGIC_ONE:");
	//TAKES 7 CYCLES TO GET HERE FROM SEND_BEGIN:
	while(TA0R < HALF_TX_PERIOD);
	DISABLE_PWM()
	TA0R = 0;
									//off for 75us

	//shift mask
	asm(" RLA	SEND_BIT_MASK");				//1

	//increase address/reset bit mask if needed
	asm(" CMP	#0, SEND_BIT_MASK");			//1
	asm(" JEQ	SEND_DELAY_RESET_B");			//2
	asm(" JMP	SEND_DELAY_RESET_POST_B");	    //2

	//otherwise shift mask and continue
	asm("SEND_DELAY_RESET_B:");
	asm(" MOV	#0001h, SEND_BIT_MASK");	//1
	asm(" ADD	#2, SEND_COMMAND");			//1
	asm("SEND_DELAY_RESET_POST_B:");

	while(TA0R < HALF_TX_PERIOD);
	ENABLE_PWM()
	TA0R = 0;
									//on for 75us

	//repeat
	asm(" JMP	SEND_BEGIN");					//2

	//send stop bit
	asm("SEND_END:");
	//send eof bitS
	while(TA0R < HALF_TX_PERIOD);
	ENABLE_PWM()
	TA0R = 0;
									//on for 75us

	while(TA0R < HALF_TX_PERIOD);
	DISABLE_PWM()
	TA0R = 0;
									//off for 75us

	while(TA0R < HALF_TX_PERIOD);
	ENABLE_PWM()
	TA0R = 0;
									//on for 263us


	while(TA0R < LONG_TX);
	DISABLE_PWM()
	TA0R = 0;


	P4OUT &= ~PWM_PIN;
	TB0CCTL1 = OUTMOD_0;
	//reset the comparator to get ready to receive the next frame.
	/*PJOUT &= ~RX_EN_PIN;
	TA0R = 0;
	while(TA0R < 60000);
	PJOUT |= RX_EN_PIN;*/

	return;
}
