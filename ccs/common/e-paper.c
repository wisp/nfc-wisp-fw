/******************************************************************************
 * e-paper.c
 *
 * @author Artem Dementyev, Pervasive Displays staff
 *
 *****************************************************************************/

#include "e-paper.h"

void Delay_us(unsigned int delay) {
	TA2CTL |= TACLR;                                    // counter reset

	TA2CCR0 = delay * 2;                                // period
	TA2CCR1 = 0;
	TA2CCR2 = 0;

	TA2CCTL0 = OUTMOD_0 | CCIS_2 | CM_0;                // OUT, Capture GND, Capture disabled
	TA2CCTL1 = OUTMOD_0 | CCIS_2 | CM_0;                // OUT, Capture GND, Capture disabled
	TA2CCTL2 = OUTMOD_0 | CCIS_2 | CM_0;                // OUT, Capture GND, Capture disabled

	TA2CTL = TASSEL_2 | ID_3 | MC_1;                    // SMCLK, /8, up

	while (0 == (TA2CTL & TAIFG)) {                     // wait for end
		//	asm ("\tnop");
	}


	TA2CTL = 0;                                         // stop the timer
}

void Delay_ms(uint16_t delay) {
	for (uint16_t i = 0; i < delay; ++i) {
		Delay_us(1000);
	}
}

//TODO verify this will work, now its 2 kHz. Data sheet says should be between 100 and 300 kHz
// TODO Why does EPD still work even when PWM is happening on the wrong line?! Fix it!
void PWM_start(void) {

		EPD_PWM_OUT &= ~EPD_PWM_BIT;
		EPD_PWM_DIR |= EPD_PWM_BIT;							// Timer_A3 outputs

		EPD_PWM_SEL |= EPD_PWM_BIT;							// ...as timer output

		TA0CTL |= TACLR;                                    // counter reset

		TA0CTL = TASSEL_2 | ID_0 | MC_3;                    // SMCLK, /1, up/down
		TA0CCR0 = TIMER1_COUNTS;                            // period square wave on TA1.0
		TA0CCR1 = TIMER1_COUNTS / 2;                        // toggle point (subtract interlock delay) TA1.1
		TA0CCR2 = TIMER1_COUNTS / 2;                        // toggle point TA1.2

		TA0CCTL0 = OUTMOD_4 | CCIS_2 | CM_0;                // Toggle, Capture GND, Capture disabled
		TA0CCTL1 = OUTMOD_3 | CCIS_2 | CM_0;                // PWM toggle/reset, Capture GND, Capture disabled
		TA0CCTL2 = OUTMOD_6 | CCIS_2 | CM_0;                // PWM toggle/set, Capture GND, Capture disabled

}

// TODO Why does EPD still work even when PWM is happening on the wrong line?! Fix it!
void PWM_stop(void) {
	EPD_PWM_OUT &= ~EPD_PWM_BIT;							// Set low
	EPD_PWM_DIR |= EPD_PWM_BIT;								// Output mode

	// TODO Is the following correct?
	EPD_PWM_SEL &= ~EPD_PWM_BIT;							// Pin function: Normal output

	TA0CTL = TACLR;                                     // counter stop
}

//Use UCB1 instead of UCB0 as in LaunchPad
void SPI_initialize(void) {

	SPI_MISO_SEL |= SPI_CLK_BIT | SPI_MISO_BIT | SPI_MOSI_BIT;	// ...as SPI

	UCB1CTL1 |= UCSWRST;                                // reset the module

	UCB1CTL0 = UCCKPH | UCMSB | UCMST | UCMODE_0 | UCSYNC;  // 8 bit, 3-wire, sync
	UCB1CTL1 = UCSSEL_2 | UCSWRST;                      // CLK = MCLK, reset the module
	UCB1BR0 = SPI_BR_LOW;                               // low byte
	UCB1BR1 = SPI_BR_HIGH;                              // high byte

	//	IE2 &= UCB0TXIE | UCB0RXIE;                         // disable interrupts
	UCB1IE &= UCTXIE | UCRXIE;							// disable interrupts

	UCB1CTL1 &= ~UCSWRST;                               // reset completed
}


void SPI_put(unsigned char c) {
	while (0 == (UCB1IFG & UCTXIFG)) {
	}
	UCB1TXBUF = c;
}

void SPI_put_wait(unsigned char c) {
	while (0 == (UCB1IFG & UCTXIFG)) {
	}
	UCB1TXBUF = c;

	// wait for last byte to clear SPI
	while (0 != (UCB1STAT & UCBUSY)) {
	}
	// wait for COG ready
	while (0 != (EPD_BUSY_IN & EPD_BUSY_BIT)) {
	}
}


void SPI_send(const unsigned char *buffer, unsigned int length) {
	// CS low
	SPI_CS_EPD_OUT &= ~SPI_CS_EPD_BIT;

	// send all data
	for (unsigned int i = 0; i < length; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = *buffer++;
	}

	// wait for last byte to clear SPI
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	// CS high
	SPI_CS_EPD_OUT |= SPI_CS_EPD_BIT;
}


void EPD_initialize(EPD_type *cog) {

	// display size dependant items
	// uint8_t *channel_select[] = {0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x00};

	unsigned int channel_select_length = 9;
	//cog->gate_source = CU8(0x72, 0x03);
	cog->gate_source_length = 2;


	cog->lines_per_display = 176;
	cog->dots_per_line = 264;
	cog->bytes_per_line = 264 / 8;
	cog->bytes_per_scan = 176 / 4;
	cog->filler = true;
	unsigned char channel_select[] = {0x72, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xfe, 0x00, 0x00};

	unsigned char cmd[] = {0x72, 0x00};

	cog->gate_source = cmd;




	// Power up sequence
	SPI_put(0x00);

	EPD_DISCH_OUT &= ~(EPD_DISCH_BIT);
	SPI_CS_EPD_OUT &= ~(SPI_CS_EPD_BIT);
	EPD_RESET_OUT &= ~(EPD_RESET_BIT);
	EPD_PANEL_ON_OUT &= ~(EPD_PANEL_ON_BIT);

	PWM_start();
	Delay_ms(5);
	EPD_PANEL_ON_OUT|= EPD_PANEL_ON_BIT;
	Delay_ms(10);
	EPD_RESET_OUT |= (EPD_RESET_BIT);
	SPI_CS_EPD_OUT |= (SPI_CS_EPD_BIT);
	Delay_ms(5);
	EPD_RESET_OUT &= ~EPD_RESET_BIT;
	Delay_ms(5);
	EPD_RESET_OUT |= EPD_RESET_BIT;
	Delay_ms(5);

	// wait for COG to become ready
	while (0 != (EPD_BUSY_IN & EPD_BUSY_BIT)) {
	}

	// channel select
	Delay_us(10);
	cmd[0] = 0x70;
	cmd[1] = 0x01;
	SPI_send(cmd, 2);
	Delay_us(10);
	SPI_send(channel_select, channel_select_length);

	// DC/DC frequency
	Delay_us(10);
	cmd[0] = 0x70;
	cmd[1] = 0x06;
	SPI_send(cmd, 2);
	Delay_us(10);

	cmd[0] = 0x72;
	cmd[1] = 0xff;
	SPI_send(cmd, 2);

	// high power mode osc
	Delay_us(10);

	cmd[0]= 0x70;
	cmd[1] = 0x07;
	SPI_send(cmd, 2);
	Delay_us(10);

	cmd[0] = 0x72;
	cmd[1] = 0x9d;
	SPI_send(cmd, 2);


	// disable ADC
	Delay_us(10);

	cmd[0] = 0x70;
	cmd[1] = 0x08;
	SPI_send(cmd, 2);
	Delay_us(10);

	cmd[0] = 0x72;
	cmd[1] = 0x00;
	SPI_send(cmd, 2);

	// Vcom level

	Delay_us(10);
	cmd[0] = 0x70;
	cmd[1] = 0x09;
	SPI_send(cmd, 2);
	Delay_us(10);

	uint8_t cmd2[] = {0x72, 0xd0, 0x00};
	SPI_send(cmd2, 3);

	// gate and source voltage levels
	Delay_us(10);
	cmd[0] = 0x70;
	cmd[1] = 0x04;
	SPI_send(cmd, 2);
	Delay_us(10);
	SPI_send(cog->gate_source, cog->gate_source_length);

	Delay_ms(5);  //???

	// driver latch on
	Delay_us(10);

	cmd[0] = 0x70;
	cmd[1] = 0x03;
	SPI_send(cmd, 2);
	Delay_us(10);
	cmd[0] = 0x72;
	cmd[1] = 0x01;
	SPI_send(cmd, 2);

	// driver latch off
	Delay_us(10);
	cmd[0] = 0x70;
	cmd[1] = 0x03;
	SPI_send(cmd, 2);
	Delay_us(10);
	cmd[0] = 0x72;
	cmd[1] = 0x00;
	SPI_send(cmd, 2);

	Delay_ms(5);

	// charge pump positive voltage on
	Delay_us(10);

	cmd[0] = 0x70;
	cmd[1] = 0x05;
	SPI_send(cmd, 2);
	Delay_us(10);
	cmd[0] = 0x72;
	cmd[1] = 0x01;
	SPI_send(cmd, 2);

	// final delay before PWM off
	Delay_ms(30);
	PWM_stop();

	// charge pump negative voltage on
	Delay_us(10);
	cmd[0] = 0x70;
	cmd[1] = 0x05;
	SPI_send(cmd, 2);
	Delay_us(10);
	cmd[0] = 0x72;
	cmd[1] = 0x03;
	SPI_send(cmd, 2);

	Delay_ms(30);

	// Vcom driver on
	Delay_us(10);
	cmd[0] = 0x70;
	cmd[1] = 0x05;
	SPI_send(cmd, 2);
	Delay_us(10);
	cmd[0] = 0x72;
	cmd[1] = 0x0f;
	SPI_send(cmd, 2);

	Delay_ms(30);

	// output enable to disable
	Delay_us(10);
	cmd[0] = 0x70;
	cmd[1] = 0x02;
	SPI_send(cmd, 2);
	Delay_us(10);
	cmd[0] = 0x72;
	cmd[1] = 0x24;
	SPI_send(cmd, 2);

}


//One frame of data is the number of lines * rows. For example:
//The 1.44” frame of data is 96 lines * 128 dots.
//The 2” frame of data is 96 lines * 200 dots.
//The 2.7” frame of data is 176 lines * 264 dots.

// the image is arranged by line which matches the display size
// so smallest would have 96 * 32 bytes

void EPD_frame_fixed(EPD_type *cog, unsigned char fixed_value) {
	for (unsigned char line = 0; line < cog->lines_per_display ; ++line) {
		EPD_line(cog, line, 0, fixed_value, EPD_normal);
	}
}



void EPD_line(EPD_type *cog, unsigned int line, const unsigned char *data, unsigned char fixed_value, EPD_stage stage) {
	// charge pump voltage levels
	Delay_us(10);
	unsigned char cmd[] = {0x70, 0x04};
	SPI_send(cmd, 2);
	Delay_us(10);
	//SPI_send(PORT2_EPD_CS, CU8(0x72, 0x03), 2);
	SPI_send(cog->gate_source, cog->gate_source_length);

	// send data
	Delay_us(10);
	cmd[0] = 0x70;
	cmd[1] = 0x0a;
	SPI_send(cmd, 2);
	Delay_us(10);

	// CS low
	SPI_CS_EPD_OUT &= ~SPI_CS_EPD_BIT;
	SPI_put_wait(0x72);

	// even pixels
	for (unsigned int b = cog->bytes_per_line; b > 0; --b) {
		if (0 != data) {
			unsigned char pixels = data[b - 1] & 0xaa;
			switch(stage) {
			case EPD_compensate:  // B -> W, W -> B (Current Image)
				pixels = 0xaa | ((pixels ^ 0xaa) >> 1);
				break;
			case EPD_white:       // B -> N, W -> W (Current Image)
				pixels = 0x55 + ((pixels ^ 0xaa) >> 1);
				break;
			case EPD_inverse:     // B -> N, W -> B (New Image)
				pixels = 0x55 | (pixels ^ 0xaa);
				break;
			case EPD_normal:       // B -> B, W -> W (New Image)
				pixels = 0xaa | (pixels >> 1);
				break;
			}
			SPI_put_wait(pixels);
		} else {
			SPI_put_wait(fixed_value);
		}
	}

	// scan line
	for (unsigned int b = 0; b < cog->bytes_per_scan; ++b) {
		if (line / 4 == b) {
			SPI_put_wait(0xc0 >> (2 * (line & 0x03)));
		} else {
			SPI_put_wait(0x00);
		}
	}

	// odd pixels
	for (unsigned int b = 0; b < cog->bytes_per_line; ++b) {
		if (0 != data) {
			unsigned char pixels = data[b] & 0x55;
			switch(stage) {
			case EPD_compensate:  // B -> W, W -> B (Current Image)
				pixels = 0xaa | (pixels ^ 0x55);
				break;
			case EPD_white:       // B -> N, W -> W (Current Image)
				pixels = 0x55 + (pixels ^ 0x55);
				break;
			case EPD_inverse:     // B -> N, W -> B (New Image)
				pixels = 0x55 | ((pixels ^ 0x55) << 1);
				break;
			case EPD_normal:       // B -> B, W -> W (New Image)
				pixels = 0xaa | pixels;
				break;
			}
			unsigned char p1 = (pixels >> 6) & 0x03;
			unsigned char p2 = (pixels >> 4) & 0x03;
			unsigned char p3 = (pixels >> 2) & 0x03;
			unsigned char p4 = (pixels >> 0) & 0x03;
			pixels = (p1 << 0) | (p2 << 2) | (p3 << 4) | (p4 << 6);
			SPI_put_wait(pixels);
		} else {
			SPI_put_wait(fixed_value);
		}
	}

	if (cog->filler) {
		SPI_put_wait(0x00);
	}

	// CS high
	SPI_CS_EPD_OUT |= SPI_CS_EPD_BIT;

	// output data to panel
	Delay_us(10);
	cmd[0] = 0x70;
	cmd[1] = 0x02;
	SPI_send(cmd, 2);
	Delay_us(10);
	cmd[0] = 0x72;
	cmd[1] = 0x2f;
	SPI_send(cmd, 2);
}


void EPD_finalise(EPD_type *cog) {

	unsigned char cmd[] = {0x70, 0x04};
	Delay_ms(25);

	// latch reset turn on
	Delay_us(10);
	cmd[0] = 0x70;
	cmd[1] = 0x03;
	SPI_send(cmd, 2);
	Delay_us(10);
	cmd[0] = 0x72;
	cmd[1] = 0x01;
	SPI_send(cmd, 2);

	// output enable off
	Delay_us(10);
	cmd[0] = 0x70;
	cmd[1] = 0x02;
	SPI_send(cmd, 2);
	Delay_us(10);
	cmd[0]= 0x72;
	cmd[1] = 0x05;
	SPI_send(cmd, 2);

	// Vcom power off
	Delay_us(10);
	cmd[0] = 0x70;
	cmd[1] = 0x05;
	SPI_send(cmd, 2);
	Delay_us(10);
	cmd[0] = 0x72;
	cmd[1] = 0x0e;
	SPI_send(cmd, 2);

	// power off negative charge pump
	Delay_us(10);
	cmd[0] = 0x70;
	cmd[1] = 0x05;
	SPI_send(cmd, 2);
	Delay_us(10);
	cmd[0]= 0x72;
	cmd[1] = 0x02;
	SPI_send(cmd, 2);

	// discharge
	Delay_us(10);
	cmd[0] = 0x70;
	cmd[1] = 0x04;
	SPI_send(cmd, 2);
	Delay_us(10);
	cmd[0] = 0x72;
	cmd[1] = 0x0c;
	SPI_send(cmd, 2);

	Delay_ms(120);

	// all charge pumps off
	Delay_us(10);
	cmd[0] = 0x70;
	cmd[1] = 0x05;
	SPI_send(cmd, 2);
	Delay_us(10);
	cmd[0] = 0x72;
	cmd[1] = 0x00;
	SPI_send(cmd, 2);

	// turn of osc
	Delay_us(10);
	cmd[0] = 0x70;
	cmd[1] = 0x07;
	SPI_send(cmd, 2);
	Delay_us(10);
	cmd[0] = 0x72;
	cmd[1] = 0x0d;
	SPI_send(cmd, 2);

	// discharge internal - 1
	Delay_us(10);
	cmd[0] = 0x70;
	cmd[1] = 0x04;
	SPI_send(cmd, 2);
	Delay_us(10);
	cmd[0] = 0x72;
	cmd[1] = 0x50;
	SPI_send(cmd, 2);

	Delay_ms(40);

	// discharge internal - 2
	Delay_us(10);
	cmd[0] = 0x70;
	cmd[1] = 0x04;
	SPI_send(cmd, 2);
	Delay_us(10);
	cmd[0] = 0x72;
	cmd[1] = 0xA0;
	SPI_send(cmd, 2);

	Delay_ms(40);

	// discharge internal - 3
	Delay_us(10);
	cmd[0] = 0x70;
	cmd[1] = 0x04;
	SPI_send(cmd, 2);
	Delay_us(10);

	cmd[0] = 0x72;
	cmd[1] = 0x00;
	SPI_send(cmd, 2);

	// turn off power and all signals
	EPD_PANEL_ON_OUT &= ~(EPD_PANEL_ON_BIT);
	//EPD_DISCH_OUT &= ~(EPD_DISCH_BIT);
	//EPD_DISCH_OUT |= EPD_DISCH_BIT;
	EPD_DISCH_OUT &= ~(EPD_DISCH_BIT);
	EPD_DISCH_OUT |= EPD_DISCH_BIT;

	SPI_CS_EPD_OUT &= ~(SPI_CS_EPD_BIT);
	EPD_RESET_OUT &= ~(EPD_RESET_BIT);
	SPI_put(0x00);

	Delay_ms(150);
	EPD_PANEL_ON_OUT &= ~(EPD_PANEL_ON_BIT);
	EPD_DISCH_OUT &= ~(EPD_DISCH_BIT);
	SPI_CS_EPD_OUT &= ~(SPI_CS_EPD_BIT);
	EPD_RESET_OUT &= ~(EPD_RESET_BIT);
}

void EPD_frame_data(EPD_type *cog, const unsigned char *image, EPD_stage stage) {
	for (unsigned char line = 0; line < cog->lines_per_display; ++line) {
		EPD_line(cog, line, &image[line * cog->bytes_per_line], 0, stage);
		//Delay_ms(20);
	}
}

void updateDisplay(unsigned char* imageBuffer, EPD_type* cog){

	LED_2_BIT_auto_pulse();

	EPD_initialize(cog);

	// TODO At some point, try sending inverted version of previous image to more efficiently clear display (like Kindle does)

	for (int16_t i = 0; i < EPD_FRAME_CYCLES; ++i) {
		EPD_frame_fixed(cog, 0xaa);  // all black
	}
	for (int16_t i = 0; i < EPD_FRAME_CYCLES; ++i) {
		EPD_frame_data(cog, imageBuffer, EPD_normal);
	}

	EPD_finalise(cog);

	LED_2_BIT_auto_pulse();
}
