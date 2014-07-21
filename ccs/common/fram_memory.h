/******************************************************************************
 * fram_memory.h
 *
 * @date Jun 15, 2013
 * @author Artem Dementyev, Aaron Parks
 *****************************************************************************/
#include <msp430.h>
#include <stdint.h>

void SPI_FRAM_Write_Enable_Latch(unsigned char portj_cs_pin);
unsigned char SPI_FRAM_Read_Status_Register(unsigned char portj_cs_pin);
void SPI_FRAM_Write_Memory(unsigned char portj_cs_pin, const unsigned char *bufferAddress, const unsigned char *bufferData, unsigned int lengthData);
unsigned char * SPI_FRAM_Read_Memory(unsigned char portj_cs_pin, const unsigned char *bufferAddress, unsigned int lengthData);
void SPI_FRAM_Enter_Sleep(void);
void SPI_FRAM_Wake_Up(unsigned char portj_cs_pin);
void SPI_FRAM_Read_Image(unsigned char portj_cs_pin, const unsigned char *bufferAddress, unsigned int lengthData);
