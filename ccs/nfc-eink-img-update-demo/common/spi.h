/******************************************************************************
 * @file	spi.h
 * @brief	spi interface
 * @author	Aaron Parks, Eve (Yi Zhao) - Sensor Systems Lab (UW)
 * @date	03/29/2015
 *****************************************************************************/

#ifndef SENSOR_SPI_H_
#define SENSOR_SPI_H_

#include <stdint.h>
//=============================================================================
//									SPI Defines
//=============================================================================
//TODO: the size of SPI depends on application
#warning "modify the size of SPI buffer for different requirement"
#define SPI_GP_RXBUF_SIZE 17

//#define CLK_MHz 13.56
//#define SPI_BPS 10000000
//#define SPI_BR_VALUE (1000000 * CLK_MHz / SPI_BPS)
//SPI CLK is 13.56 Mhz
//Baudrate now is 13.56Mhz/SPI_BR_LOW = 4.39Mhz
#define SPI_BR_HIGH  0x00
#define SPI_BR_LOW   0x03//0x03

//Define index for different SPI slave
//TODO: Add more depends on different SPI applications
#define FRAM_SPI	 	0x00
#define EPD_SPI      	0x01
#define ACCEL_SPI    	0x02
#define EPD_LONG_DATA 	0x03


//=============================================================================
//									SPI Micros
//=============================================================================
//SPI_CS_xx configureation used in SPI_transaction
#define	MEM_SPI_ON()  \
			PSEL_SPI |= (SPI_CLK_BIT | SPI_MOSI_BIT | SPI_MISO_BIT); \
			SPI_CS_MEM_OUT &= ~SPI_CS_MEM_BIT;
//reset CLK,MOSI to be 0/output MISO input and SPI_CS to be 1
#define	MEM_SPI_OFF()	 \
			POUT_SPI &= ~(SPI_CLK_BIT | SPI_MOSI_BIT); \
			PDIR_SPI &= ~(SPI_MISO_BIT); \
			PDIR_SPI |=  (SPI_CLK_BIT | SPI_MOSI_BIT); \
			SPI_CS_MEM_OUT |= SPI_CS_MEM_BIT;		   \
			PSEL_SPI &= ~(SPI_CLK_BIT | SPI_MOSI_BIT | SPI_MISO_BIT);

#define EPD_SPI_ON() \
			SPI_CS_EPD_OUT &=~ SPI_CS_EPD_BIT;
#define EPD_SPI_OFF() \
			SPI_CS_EPD_OUT |=  SPI_CS_EPD_BIT;
#define ACCEL_SPI_ON() \
			PSEL_SPI |= (SPI_CLK_BIT | SPI_MOSI_BIT | SPI_MISO_BIT); \
			SPI_CS_ACCEL_OUT &=~ SPI_CS_ACCEL_BIT;
#define ACCEL_SPI_OFF() \
			SPI_CS_ACCEL_OUT |=  SPI_CS_ACCEL_BIT;	   \
			POUT_SPI &= ~(SPI_CLK_BIT | SPI_MOSI_BIT); \
			PDIR_SPI &= ~(SPI_MISO_BIT); \
			PDIR_SPI |=  (SPI_CLK_BIT | SPI_MOSI_BIT); \
			PSEL_SPI &= ~(SPI_CLK_BIT | SPI_MOSI_BIT | SPI_MISO_BIT);

//=============================================================================
//								Variables
//=============================================================================


extern uint8_t gpRxBuf[SPI_GP_RXBUF_SIZE];
//extern uint8_t gpTxBuf[SPI_GP_RXBUF_SIZE];
typedef uint8_t  BOOL;


//=============================================================================
//									SPI Functions
//=============================================================================
BOOL SPI_initialize();
BOOL SPI_acquirePort();
BOOL SPI_releasePort();
void SPI_transaction(uint8_t* rxBuf, uint8_t* txBuf, uint16_t size, uint8_t spiSlaveIndex);
void SPI_put_wait(unsigned char c);
void epd_spi_send_byte (const uint8_t register_index, const uint8_t register_data);
void epd_spi_send (uint8_t register_index, uint8_t *dataPtr, uint8_t length);
uint8_t SPI_READ(const uint8_t Register);


#endif /* SENSOR_SPI_H_ */
