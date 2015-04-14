/******************************************************************************
 * @file	e-paper.c
 *
 * @brief	Functions for update Gen2 e-paper (can update whole screan or one dot)
 *
 * @details This driver is modified for low-power operation
 * 			Reference: the COG Gen2 Driver Interface Timing (for EM027BS013)
 * 			Tested on E-paper (EM027BS013/E1200CS021-ND)
 *
 * @note 	More details about driver reference can be found
 * 			http://www.pervasivedisplays.com/products/27
 * 			Modify EPD_type cog, EPD_initial_driver and EPD_off() for other EPD
 *
 * @author	Eve (Yi Zhao) - Sensor Systems Lab (UW)
 * @date	03/29/2015
 *****************************************************************************/


//=============================================================================
//									Includes
//=============================================================================
#include "e-paper.h"
#include "timer.h"


//=============================================================================
//									Globals
//=============================================================================
uint8_t 			imageBuffer[E_INK_SIZE];
uint8_t volatile 	imageUpdateState;



#if defined(E_INK_2_0)
	/*******Values of EPD_type for  2" EPD Panel*******/
	const struct EPD_type cog = {
			{0x00,0x00,0x00,0x00,0x01,0xFF,0xE0,0x00}, //< the SPI register data of Channel Select
			0x03,									   //< the SPI register data of Voltage Level
			(200/8),  								   //< the bytes of width of EPD
			96, 									   //< the bytes of height of EPD
			((((200+96)*2)/8)+1),			           //< Data + Scan + Dummy bytes
			(96/4)								   		//<Bytes for store >
	};
#elif defined(E_INK_2_7)
	/*******Values of EPD_type for  2.7" EPD Panel*******/
	const struct EPD_type cog = {
			{0x00,0x00,0x00,0x7F,0xFF,0xFE,0x00,0x00}, //< the SPI register data of Channel Select
			0x00,									   //< the SPI register data of Voltage Level
			(33),//264/8							   //< the bytes of width of EPD
			176,									   //< the bytes of height of EPD
			(111),//((((264+176)*2)/8)+1)			   //< Data + Scan + Dummy bytes
			(44)//176/4								   //<Bytes for store >
	};

#endif

/*******Values of EPD_type for  1.4" EPD Panel*******/
/*
const struct EPD_type cog = {
		{0x00,0x00,0x00,0x00,0x00,0x0F,0xFF,0x00}, //< the SPI register data of Channel Select
		0x03,									   //< the SPI register data of Voltage Level
		(128/8),								   //< the bytes of width of EPD
		96,									   	   //< the bytes of height of EPD
		((((128+96)*2)/8)+1),	 				   //< Data + Scan + Dummy bytes
			(96/4)								   //<Bytes for store >
};
*/
//const EPD_type* cogPtr = &cog;

/*The stage during frame update according to e-paper COG Driver datasheet*/
typedef enum {           // Image pixel -> Display pixel
	EPD_compensate,  // B -> W, W -> B (Current Image)
	EPD_white,       // B -> N, W -> W (Current Image)
	EPD_inverse,     // B -> N, W -> B (New Image)
	EPD_normal       // B -> B, W -> W (New Image)
} EPD_stage;


//=============================================================================
//									Local Variables
//=============================================================================
static const uint8_t   SCAN_TABLE[4] = {0x03,0x0c,0x30,0xC0};

/**
*	@brief: Power up E-paper IC dirver
*	@function
*	@time 	15ms
*/

//=============================================================================
//									Micros
//=============================================================================
void EPD_power_on(void){
		//UCSCTL8	&= ~(MCLKREQEN|ACLKREQEN);
		EPD_BUSY_DIR &=~ EPD_BUSY_BIT;
		EPD_PANEL_ON_OUT|= EPD_PANEL_ON_BIT;
		SPI_CS_EPD_OUT |= (SPI_CS_EPD_BIT);
		SPI_CS_EPD_DIR |= (SPI_CS_EPD_BIT);
		PSEL_SPI |= (SPI_CLK_BIT | SPI_MOSI_BIT | SPI_MISO_BIT); \
		EPD_RESET_OUT |= (EPD_RESET_BIT);
		lowPowerSleep(LPM_5ms);
		EPD_RESET_OUT &= ~EPD_RESET_BIT;
		lowPowerSleep(LPM_5ms);
		EPD_RESET_OUT |= EPD_RESET_BIT;
		lowPowerSleep(LPM_5ms);
}

//=============================================================================
//									Functions
//=============================================================================
/**
*	@brief:	Power off E-paper IC dirver and reset statemachine
*	@function
*	@time 	200ms
*/
uint8_t EPD_power_off(void) {
	uint8_t y;
	lowPowerSleep(LPM_40ms);
	//Check DC/DC
	//if(!(SPI_R(0x0F) & 0x40)) return ERROR_DC;
	//Turn on Latch Reset
	epd_spi_send_byte (0x03, 0x01);
	//Turn off OE
	epd_spi_send_byte (0x02, 0x05);
	//Power off charge pump Vcom
	epd_spi_send_byte (0x05, 0x0E);
	//Power off charge pump neg voltage
	epd_spi_send_byte (0x05, 0x02);
	//Turn off all charge pump
	epd_spi_send_byte (0x05, 0x00);
	//Turn off OSC
	epd_spi_send_byte (0x07, 0x0D);
	//Discharge internal-@modifyed
	epd_spi_send_byte (0x04, 0x83);
	lowPowerSleep(LPM_40ms);
	//Turn off Discharge internal
	epd_spi_send_byte (0x04, 0x00);
	//@modified increase the delay here can increase the quality of
	lowPowerSleep(LPM_120ms);
	//lowPowerSleep(LPM_40s);
	initEPD();

	//Enternal discharge @modified
	for(y=0;y<1;y++)
	{
		EPD_DISCH_OUT |=  (EPD_DISCH_BIT);
		lowPowerSleep(LPM_5ms);//@try to remove
		EPD_DISCH_OUT &= ~(EPD_DISCH_BIT);
		lowPowerSleep(LPM_5ms);//@try to remove
	}
	return RES_OK;
}


/**
*	@brief	initialize E-paper IC state machine
*	@fun	Initialize Gen2 EPD 2.7"/2.0"
*	@time 	125ms
*/
uint8_t EPD_initialize_driver (void){
		uint8_t i;
		while(EPD_BUSY_IN & EPD_BUSY_BIT){
			if((i++) >= 0x0FFF) return ERROR_BUSY;
			lowPowerSleep(LPM_50us);
		}
		//Check COG ID
		//TODO write test code for drive both Gen2 and Gen1
		//if((SPI_R(0x72,0x00) & 0x0f) !=0x02) return ERROR_COG_ID;
		//Disable OE
		epd_spi_send_byte(0x02,0x40);

		//Check Breakage
		if(!(SPI_READ(0x0F) & 0x80)) return ERROR_BREAKAGE;

		//Power Saving Mode
	 	epd_spi_send_byte(0x0B, 0x02);

		//Channel Select
		epd_spi_send (0x01,(uint8_t*)cog.channel_select, 8);

		//High Power Mode Osc Setting
		epd_spi_send_byte(0x07,0xD1);

		//Power Setting
		epd_spi_send_byte(0x08,0x02);

		//Set Vcom level
		epd_spi_send_byte(0x09,0xC2);

		//Power Setting
		epd_spi_send_byte(0x04,0x03);

		//Driver latch on
		epd_spi_send_byte(0x03,0x01);

		//Driver latch off
		epd_spi_send_byte(0x03,0x00);

		lowPowerSleep(LPM_5ms);

		//Chargepump Start
		i=1;
		do {
			//Start chargepump positive V
			//VGH & VDH on
			epd_spi_send_byte(0x05,0x01);

			//@modified from 240 to 40ms
			lowPowerSleep(LPM_40ms);
			//lowPowerSleep(LPM_10ms);
			//NOTE: maybe we can read data from FRAM to replace this lowPowerDelay here

			//Start chargepump neg voltage
			//VGL & VDL on
			epd_spi_send_byte(0x05,0x03);

			lowPowerSleep(LPM_20ms);

			//Set chargepump
			//Vcom_Driver to ON
			//Vcom_Driver on
			epd_spi_send_byte(0x05,0x0F);

			lowPowerSleep(LPM_40ms);

			//Check DC/DC
			if((SPI_READ(0x0F) & 0x40) == 0x40) break;
			i++;
		}while(i<4);//NOTE: Suggested to be 4 in datasheet use 1 here works
		if(i==4)return ERROR_CHARGEPUMP;
		//epd_spi_send_byte(0x02,0x40);

		/*Correct OE seting based on the latest datasheet of COG Gen2 driver*/
		epd_spi_send_byte(0x02,0x04);	//@modify based on latest datasheet
		return RES_OK;
}

uint8_t  EPD_power_init(void) {
	EPD_power_on();
	return EPD_initialize_driver();
}


/**
*	@fun	Update single byte of dot of choosing line
*	@papra	lines is the updated lines,
*			byte_pos is the byte position in one line
*			fixed_value is the color of 4 pixel in that byte;
*/
void EPD_dot(uint8_t lines, uint8_t byte_pos, const uint8_t value){
	uint8_t lines_byte,b,RxData[2],pixels;
	const uint8_t Index[2]= {0x70,0x0A};

	//send header
	while (EPD_BUSY_IN & EPD_BUSY_BIT){
				delay_us(T_10us);}

	delay_us(T_10us);
	SPI_transaction(RxData,(uint8_t *)Index,2,EPD_SPI);
	// send data
	delay_us(T_10us);

	SPI_CS_EPD_OUT &= ~SPI_CS_EPD_BIT;
	SPI_put_wait(0x72);
	SPI_put_wait(0x00);
	// Odd pixels start from p263
	for (b = cog.bytes_per_line; b > 0; b--) {
		if ((b-1)==byte_pos) {
			unsigned char temp_byte = value;
			pixels  = ((temp_byte & 0x80) ? WHITE0  : BLACK0 );
			pixels |= ((temp_byte & 0x20) ? WHITE1  : BLACK1 );
			pixels |= ((temp_byte & 0x08) ? WHITE2  : BLACK2 );
			pixels |= ((temp_byte & 0x02) ? WHITE3  : BLACK3 );
			SPI_put_wait(pixels);
		}else{
			SPI_put_wait(0x00);
		}
	}

	// scan line
	//from S(176) to S(1)
	lines_byte = (lines>>2)+1;

	//for (b = 0; b <cog.bytes_per_scan; b++) {
	for (b = cog.bytes_per_scan; b >0; b--) {
		if (b == lines_byte) {
			SPI_put_wait(SCAN_TABLE[lines&0x03]);
			//SPI_put_wait(0xFF);
		} else {
			SPI_put_wait(0x00);
		}
	}

	// Even pixels
	//start from pixel 2 (the first pixel is 1)
	for (b = 0; b < cog.bytes_per_line; b++) {
		if (b==byte_pos) {//LSB
			unsigned char temp_byte = value;
			pixels  = ((temp_byte & 0x01) ? WHITE0  : BLACK0 );
			pixels |= ((temp_byte & 0x04) ? WHITE1  : BLACK1 );
			pixels |= ((temp_byte & 0x10) ? WHITE2  : BLACK2 );
			pixels |= ((temp_byte & 0x40) ? WHITE3  : BLACK3 );
			SPI_put_wait(pixels);
		}else{
			SPI_put_wait(0x00);
		}
	}

	// CS high
	SPI_CS_EPD_OUT |= SPI_CS_EPD_BIT;

	//Turn on OE
	epd_spi_send_byte(0x02,0x07);
}

/**
*	@fun	Update choosing line of E-ink
*	@papra	lines is the updated lines,
*			data is the content of image for updated line
*			fixed_value is update line with ALL_WHITE or ALL_BLACK
*			stage  we didn't implement stage based on E-ink datasheet
*/
void EPD_line(uint8_t lines, uint8_t* data, uint8_t fixed_value, EPD_stage stage){
	uint8_t lines_byte,b,RxData[2],pixels;
	const uint8_t Index[2]= {0x70,0x0A};

	//send header
	while (EPD_BUSY_IN & EPD_BUSY_BIT){
				delay_us(T_10us);}

	delay_us(T_10us);
	SPI_transaction(RxData,(uint8_t*)Index,2,EPD_SPI);
	// send data
	delay_us(T_10us);
	SPI_CS_EPD_OUT &= ~SPI_CS_EPD_BIT;
	SPI_put_wait(0x72);
	SPI_put_wait(0x00);
	// Odd pixels start from p263
	for (b = cog.bytes_per_line; b > 0; b--) {
		if (data) {
			unsigned char temp_byte = data[b - 1];
			switch(stage){
				case EPD_inverse:
					pixels  = ((temp_byte & 0x80) ? BLACK0 : WHITE0);
					pixels |= ((temp_byte & 0x20) ? BLACK1 : WHITE1);
					pixels |= ((temp_byte & 0x08) ? BLACK2 : WHITE2 );
					pixels |= ((temp_byte & 0x02) ? BLACK3 : WHITE3 );
					break;
				case EPD_normal://MSB
					pixels  = ((temp_byte & 0x80) ? WHITE0  : BLACK0 );
					pixels |= ((temp_byte & 0x20) ? WHITE1  : BLACK1 );
					pixels |= ((temp_byte & 0x08) ? WHITE2  : BLACK2 );
					pixels |= ((temp_byte & 0x02) ? WHITE3  : BLACK3 );
					break;

			}
			SPI_put_wait(pixels);
		} else {
			SPI_put_wait(fixed_value);
		}
	}

	// scan line
	//from S(176) to S(1)
	lines_byte = (lines>>2)+1;
	//for (b = 0; b <cog.bytes_per_scan; b++) {
	for (b = cog.bytes_per_scan; b >0; b--) {
		if (b == lines_byte) {
			SPI_put_wait(SCAN_TABLE[lines&0x03]);
			//SPI_put_wait(0xFF);
		} else {
			SPI_put_wait(0x00);
		}
	}

	// Even pixels
	//start from pixel 2 (the first pixel is 1)
	for (b = 0; b < cog.bytes_per_line; b++) {
		if (data) {//LSB
			unsigned char temp_byte = data[b];
			switch(stage){
				case EPD_inverse:
					pixels  = ((temp_byte & 0x01) ? BLACK0 : WHITE0);
					pixels |= ((temp_byte & 0x04) ? BLACK1 : WHITE1);
					pixels |= ((temp_byte & 0x10) ? BLACK2 : WHITE2 );
					pixels |= ((temp_byte & 0x40) ? BLACK3 : WHITE3 );
					break;
				case EPD_normal:
					pixels  = ((temp_byte & 0x01) ? WHITE0  : BLACK0 );
					pixels |= ((temp_byte & 0x04) ? WHITE1  : BLACK1 );
					pixels |= ((temp_byte & 0x10) ? WHITE2  : BLACK2 );
					pixels |= ((temp_byte & 0x40) ? WHITE3  : BLACK3 );
					break;
			}
			SPI_put_wait(pixels);
		} else {
			SPI_put_wait(fixed_value);
		}
	}

	// CS high
	SPI_CS_EPD_OUT |= SPI_CS_EPD_BIT;
	//Turn on OE
	epd_spi_send_byte(0x02,0x07);
}

/**
*	@brief	update EPD display for multiple lines or whole screen using EPD_line
*	@param	imgPtr - the pointer pointed to the start of updated content
*			start - the starting line where the update is start from
*			end - the number of lines which is to update
*			start and start+end must less than cog.lines_per_display
*
*/
void EPD_frame_fixed(const uint8_t start, const uint8_t end, const uint8_t color) {
	uint8_t i,j;
	//uint8_t* line_ptr;
	//Stage 2
	for (j = 0; j <SINGLE_COLOR_CYCLES ; j++){
		for (i = start; i < end; i++) {
			EPD_line(i,(void *)0,color,EPD_normal);
		}
	}
}

/**
*	@brief	update EPD display with monocolor for multiple lines or whole screen
*	@param	imgPtr - the pointer pointed to the start of updated content
*			start - the starting line where the update is start from
*			end - the number of lines which is to update
*			SINGLE_COLOR_CYCES,NEW_IMG_CYCLES - the number of frame for updating
*			can be modified in the e-paper.h
*			according differnet image quality requirement
*	@note	we only update ALL_BACLK once and then the new image@modified
*	@note	NFC-WISP Reader App 1.0 will sending dummy data for power
*			after finishing image transfer.
*			If you build *BUILD_E_INK_2_0_BATT_FREE*
*			we add more lowPowerSleep during E-ink updating to reive more energy from cell-phone
*/
void EPD_frame_newImg(uint8_t* imgPtr,const uint8_t start, const uint8_t end) {
	uint8_t i,j;
	uint8_t* line_ptr;
	//Stage 1
	/*
	for (j = 0; j < NEW_IMG_CYCLES; j++) {
				line_ptr = imgPtr;
				for (i = start; i < end; i++) {
					//for (j = 0; j < NEW_IMG_CYCLES; j++) {
						EPD_line(i,line_ptr,0, EPD_inverse);
					    line_ptr = line_ptr + cog.bytes_per_line;
				}
		}

	*/
	//Stage 2 @modified
	for(j=0; j < SINGLE_COLOR_CYCLES; j++) {
		#ifdef BATT_FREE
			lowPowerSleep(LPM_80ms);
		#endif
			EPD_frame_fixed(start,end,ALL_BLACK);
		#ifdef BATT_FREE
			lowPowerSleep(LPM_80ms);
		#endif
	}


	//Stage 3
	for (j = 0; j < NEW_IMG_CYCLES; j++) {
			line_ptr = imgPtr;
			for (i = start; i < end; i++) {
				EPD_line(i,line_ptr,0, EPD_normal);
				line_ptr = line_ptr + cog.bytes_per_line;
			}
		#ifdef BATT_FREE
			lowPowerSleep(LPM_20ms);
		#endif
	}

}


/**
*	@brief	update EPD display with monocolor for multiple lines or whole screen
*	@param
*			start - the starting line where the update is start from
*			end - the number of lines which is to update
*			byte_pos - the location of the target 4 pixels
*			SINGLE_COLOR_CYCES,NEW_IMG_CYCLES - the number of frame for updating
*			can be modified in the e-paper.h for differnet image quality requirement
*	@note	we only update ALL_BACLK once and then the new image@modified
*/
void EPD_frame_singleDot(const uint8_t start, const uint8_t end, uint8_t byte_pos,const uint8_t value) {
	uint8_t i,j;
	//uint8_t* line_ptr;
	//Stage 1
	/*
	for (j = 0; j < NEW_IMG_CYCLES; j++) {
				line_ptr = imgPtr;
				for (i = start; i < end; i++) {
					//for (j = 0; j < NEW_IMG_CYCLES; j++) {
						EPD_line(i,line_ptr,0, EPD_inverse);
					    line_ptr = line_ptr + cog.bytes_per_line;
				}
		}

	*/
	//Stage 2
	//EPD_frame_fixed(start,end,ALL_BLACK);
	//uint8_t byte_pos, const uint8_t fixed_value

	//Stage 3
	//for (j = 0; j < NEW_IMG_CYCLES; j++) {
	for (j = 0; j < SINGLE_DOT_CYCLES; j++) {
			//line_ptr = imgPtr;
			for (i = start; i < end; i++) {
				EPD_dot(i,byte_pos,value);
				//line_ptr = line_ptr + cog.bytes_per_line;
			}
	}
}



