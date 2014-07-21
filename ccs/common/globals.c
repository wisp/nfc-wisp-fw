/*****************************************************************************
 * globals.c
 *
 * @date Mar 11, 2013
 * @author Jeremy Gummeson, Derek Thrasher (UMass Amherst)
 *****************************************************************************/

#include "globals.h"
#include "fram_memory.h"

void system_initialize()
{



	WDTCTL = WDTPW + WDTHOLD; 		// Disable watchdog timer
	_bic_SR_register(GIE); 			//Turn off GIE while we initialize.
	bad_interrupts = 0;

	////////////////////////////////////////////////
	// Turn LDO off
	////////////////////////////////////////////////
    LDOKEYPID = LDOKEY;
    LDOPWRCTL = 0x00;
    LDOKEYPID = 0;

	////////////////////////////////////////////////
	// Turn off high/low voltage supervisors
	////////////////////////////////////////////////
    PMMCTL0_H = PMMPW_H;
    SVSMHCTL &= ~(SVMHE+SVSHE);
    SVSMLCTL &= ~(SVMLE+SVSLE);


	////////////////////////////////////////////////
	// GPIO configuration
	////////////////////////////////////////////////

    // Port sel bits for relevant ports
	// P4SEL = SPI_CLK_BIT | SPI_MOSI_BIT | SPI_MISO_BIT | RX_UART_BIT;
	P5SEL = XT1IN_BIT | XT1OUT_BIT | XT2IN_BIT | XT2OUT_BIT;

    // Initial output register values
    P1OUT = 0;
    P2OUT = 0;
    P4OUT = SPI_CS_MEM_BIT;
    P5OUT = RX_ENABLE_BIT;
    P6OUT = RX_WAKEUP_EN_BIT;
    PJOUT = 0;

    // Drive strength selection for where it counts
    P5DS = RX_ENABLE_BIT;

    // Set all GPIO to output mode UNLESS they are inputs (for lowest power)
    P1DIR = ~(SW_1_BIT | RX_WAKEUP_BIT | RX_BIT | SW_2_BIT);
    P2DIR = ~(0);
    P4DIR = ~(SPI_MOSI_BIT | SPI_CLK_BIT | SPI_MISO_BIT | EPD_BUSY_BIT | RX_UART_BIT);
    P5DIR = ~(XT1IN_BIT | XT1OUT_BIT | XT2IN_BIT | XT2OUT_BIT);
    P6DIR = ~(BATT_MEAS_BIT);
    PJDIR = ~(0);

    // Interrupt edge selects
    P1IES |= (RX_BIT);

	////////////////////////////////////////////////
	// Clock configuration
	////////////////////////////////////////////////

    // Disallow requests for MCLK/SMCLK to stay on.
    // NOTE: You must let ACLK request stay on for timeout system
    UCSCTL8 &= ~(SMCLKREQEN | MCLKREQEN);

    UCSCTL6 &= ~XT2OFF; // Make sure XT2 is ON
    UCSCTL3 |= SELREF_2; // FLLref = REFO
    UCSCTL4 |= SELA_2; // ACLK=REFO,SMCLK=DCO,MCLK=DCO During initialization

    // Loop until XT1,XT2 & DCO stabilizes - in this case loop until XT2 settles
    do
    {
    	UCSCTL7 &= ~(XT2OFFG | XT1LFOFFG | DCOFFG); // Clear XT2,XT1,DCO fault flags
    	SFRIFG1 &= ~OFIFG;						// Clear fault flags
    }while (SFRIFG1&OFIFG);					// Test oscillator fault flag

    UCSCTL6 &= ~XT2DRIVE1;					// Decrease XT2 Drive according to expected frequency

    UCSCTL4 |= SELS_5 | SELM_5 | SELA_0;		// SMCLK=MCLK=XT2, ACLK=XT1

    UCSCTL7 = 0; //clear fault flags



}

/**
 * Configure LED 1 to pulse once without further intervention
 */
void led_1_auto_pulse(void) {
	led_pulse_flags|=LED_1_PULSE_FLAG;
}

/**
 * Configure LED 2 to pulse once without further intervention
 */
void LED_2_BIT_auto_pulse(void) {
	led_pulse_flags|=LED_2_BIT_PULSE_FLAG;
}

//void timeout_initialize(uint16_t timeout_val)
//{
//	//configure timeout timer (Timer A1)
//		TA1CTL	|=  TASSEL_1;	//external 32.768 KHz real-time clock. (XT1CLK by default)
//
//		//Based on observed polling behavior from phone, timeout should be ~ 100 ms in length
//		//(conservative upperbound. Can be made smaller.)
//		TA1CCR1 = timeout_val; // # timer ticks in 100 ms
//
//		TA1CCTL1 |= CCIE; //enable the timeout interrupt.
//}


