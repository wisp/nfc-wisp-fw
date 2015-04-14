/******************************************************************************
 * e-paper.h
 *
 * @bref	 EPD(E-paer Driver) COG drive for Gen2 EPD EM027BS013
 * @note	 Modify cog for different EPD size of display (must using Gen2 COG)
 *
 *****************************************************************************/

#include <msp430.h>
#include <stdint.h>
#include "../UserApp/myE-paperApp.h"
//#include "tempSense.h"


//------------------------------------------------------------------------------
//							Defines
//------------------------------------------------------------------------------
/*******E_INK image buffer size *******/
/*
 * Max should be
 * cog.bytes_per_line * cog.lines_per_display
 * @note: adjust the cog in e-paper.c to match the type of e-paper
 *
 * 176*33 = 5808 for 2.7" e-paper
 * 96*200/8 	 for 2.7" e-paper
 * 96*128/8 	 for 1.4" e-paper
 */
#if	defined(E_INK_2_0)
	#define E_INK_SIZE 		96*200/8 //2.0 e-paper
#elif defined(E_INK_2_7)
	#define E_INK_SIZE 		5808
#endif


/*******Update cycles for image *******/
//@note The large cycle number results in better image
#warning "if the image update quality is not good, then increase the NEW_IMG_CYCLES(need more power)"
#define NEW_IMG_CYCLES 		4
#define SINGLE_COLOR_CYCLES 1
#define SINGLE_DOT_CYCLES 	4

/************E-ink Update state for variable imageState************************/
#define IMG_HALT			0
#define IMG_UPDATE			1
#define SENSE_UPDATE		2
#define IMG_FORCE_UPDATE    3
#define IMG_UPDATE_COMPLETE	4
/*
 * The definition for driving stage to compare with
 * for getting Odd and Even data
 */
#define BLACK0   (uint8_t)(0x03) /**< getting bit1 or bit0 as black color(11) */
#define BLACK1   (uint8_t)(0x0C) /**< getting bit3 or bit2 as black color(11) */
#define BLACK2   (uint8_t)(0x30) /**< getting bit5 or bit4 as black color(11) */
#define BLACK3   (uint8_t)(0xC0) /**< getting bit7 or bit6 as black color(11) */
#define WHITE0   (uint8_t)(0x02) /**< getting bit1 or bit0 as white color(10) */
#define WHITE1   (uint8_t)(0x08) /**< getting bit3 or bit2 as white color(10) */
#define WHITE2   (uint8_t)(0x20) /**< getting bit5 or bit4 as white color(10) */
#define WHITE3   (uint8_t)(0x80) /**< getting bit7 or bit6 as white color(10) */
#define NOTHING0 (uint8_t)(0x01) /**< getting bit1 or bit0 as nothing input(01) */
#define NOTHING1 (uint8_t)(0x04) /**< getting bit3 or bit2 as nothing input(01) */
#define NOTHING2 (uint8_t)(0x10) /**< getting bit5 or bit4 as nothing input(01) */
#define NOTHING3 (uint8_t)(0x40) /**< getting bit7 or bit6 as nothing input(01) */
#define NOTHING  (uint8_t)(0x55) /**< sending Nothing frame, 01=Nothing, 0101=0x5 */


#define ALL_BLACK	 	 (uint8_t)(0xFF)
#define ALL_WHITE		 (uint8_t)(0xAA)
#define BORDER_BYTE_B    (uint8_t)(0xFF)
#define BORDER_BYTE_W    (uint8_t)(0xAA)
#define ERROR_BUSY       (uint8_t)(0xF0)
#define ERROR_COG_ID     (uint8_t)(0xF1)
#define ERROR_BREAKAGE   (uint8_t)(0xF2)
#define ERROR_DC         (uint8_t)(0xF3)
#define ERROR_CHARGEPUMP (uint8_t)(0xF4)
#define RES_OK           (uint8_t)(0x00)


//------------------------------------------------------------------------------
//							Micros
//------------------------------------------------------------------------------
#define initEPD() \
        POUT_SPI &= ~(SPI_CLK_BIT | SPI_MOSI_BIT | SPI_CS_EPD_BIT); \
		PDIR_SPI |=  (SPI_CLK_BIT | SPI_MOSI_BIT | SPI_CS_EPD_BIT); \
		PDIR_SPI &= ~(SPI_MISO_BIT); \
		PSEL_SPI &= ~(SPI_CLK_BIT | SPI_MOSI_BIT | SPI_MISO_BIT);	\
		EPD_RESET_OUT &= ~(EPD_RESET_BIT); \
		EPD_PANEL_ON_OUT &= ~(EPD_PANEL_ON_BIT);\

/**
 * \brief Define the COG driver's parameters */
struct EPD_type{
	uint8_t   channel_select[8]; /**< SPI register data of Channel Select */
	uint8_t   voltage_level;     /**< SPI register data of Voltage Level */
	uint8_t   bytes_per_line;   /**< bytes of width of EPD */
	uint8_t   lines_per_display;     /**< the bytes of height of EPD */
	uint8_t   data_line_size;    /**< Data + Scan + Dummy bytes */
	uint8_t	  bytes_per_scan;
};

//------------------------------------------------------------------------------
//							globals
//------------------------------------------------------------------------------
extern uint8_t 			imageBuffer[E_INK_SIZE];
extern uint8_t volatile imageUpdateState;
//extern EPD_type;
extern const struct EPD_type cog;

//------------------------------------------------------------------------------
//							Functions
//------------------------------------------------------------------------------
//Initial EPD Dirver
uint8_t  EPD_power_init(void);
//Turn off EPD Driver
uint8_t EPD_power_off(void);
//Update one line

//Update multiple lines to all White or all Black
void EPD_frame_fixed(const uint8_t start, const uint8_t end, const uint8_t color);
//Update multiple lines of new image
void EPD_frame_newImg(uint8_t* imgPtr,const uint8_t start, const uint8_t end);
//Update singleDot (4 pixels) in E-ink display
void EPD_frame_singleDot(const uint8_t start, const uint8_t end, uint8_t byte_pos,const uint8_t value);
