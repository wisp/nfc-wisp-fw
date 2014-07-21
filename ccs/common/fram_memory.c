/******************************************************************************
 * fram_memory.c
 *
 * @date Jun 15, 2013
 * @author Artem Dementyev, Aaron Parks
 *****************************************************************************/

#include "fram_memory.h"
#include "../common/e-paper.h"

void SPI_FRAM_Wake_Up() {
	// CS low
	SPI_CS_MEM_OUT &= ~SPI_CS_MEM_BIT;
	Delay_us(10);

	Delay_us(500);

	// CS high
	SPI_CS_MEM_OUT |= SPI_CS_MEM_BIT;
}

void SPI_FRAM_Write_Enable_Latch() {
	// CS low
	SPI_CS_MEM_OUT &= ~SPI_CS_MEM_BIT;

	//send the op code
	for (uint16_t i = 0; i < 1; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = 0x06; //op code for enabling write latch (WREN)
	}

	// wait for last byte to clear SPI
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	// CS high
	SPI_CS_MEM_OUT |= SPI_CS_MEM_BIT;
}

void SPI_FRAM_Enter_Sleep() {
	// CS low
	SPI_CS_MEM_OUT &= ~SPI_CS_MEM_BIT;

	//send the op code
	for (uint16_t i = 0; i < 1; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = 0xB9; //op code for entering sleep (SLEEP)
	}

	// wait for last byte to clear SPI
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	// CS high
	SPI_CS_MEM_OUT |= SPI_CS_MEM_BIT;
}

uint8_t SPI_FRAM_Read_Status_Register() {

	uint8_t temp = 0x00;

	// CS low
	SPI_CS_MEM_OUT &= ~SPI_CS_MEM_BIT;

	// send the op code
	for (uint16_t i = 0; i < 1; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = 0x05; //op code for reading status register (RDSR)
	}

	// wait for last byte to clear SPI
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	//send dummy data, to clock out the data
	for (uint16_t i = 0; i < 1; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = 0xff; //all 1's. Its dummy data
	}

	// wait for last byte to clear SPI
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	// CS high
	SPI_CS_MEM_OUT |= SPI_CS_MEM_BIT;

	temp = UCB1RXBUF;
	return (temp);
	//return RXBUF0;
}

void SPI_FRAM_Write_Memory(const uint8_t *bufferAddress,
		const uint8_t *bufferData, uint16_t lengthData) {

	// CS low
	SPI_CS_MEM_OUT &= ~SPI_CS_MEM_BIT;

	// send the op code
	for (uint16_t i = 0; i < 1; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = 0x02; //op code for writing memory (WRITE)
	}

	// wait for last byte to clear SPI
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	//send the 18-bit address of first data byte (we use 8 * 3 = 24 bits to represent address)
	for (uint16_t i = 0; i < 3; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = *bufferAddress++;
	}

	// send the 8-byte data
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	for (uint16_t i = 0; i < lengthData; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = *bufferData++;
	}

	// wait for last byte to clear SPI
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	// CS high
	SPI_CS_MEM_OUT |= SPI_CS_MEM_BIT;

} //end SPI write memory

uint8_t * SPI_FRAM_Read_Memory(const uint8_t *bufferAddress,
		uint16_t lengthData) {

	// TODO We don't have space in RAM for the following array.
	uint8_t testArray[5807];

	// CS low
	SPI_CS_MEM_OUT &= ~SPI_CS_MEM_BIT;

	// send the op code
	for (uint16_t i = 0; i < 1; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = 0x03; //op code for reading memory (READ)
	}

	// wait for last byte to clear SPI
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	//send the 18-bit address
	for (uint16_t i = 0; i < 3; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = *bufferAddress++;
	}

	// clock out the 8-byte data
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	for (uint16_t i = 0; i < lengthData; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = 0xff;
		testArray[i] = UCB1RXBUF;
	}

	// wait for last byte to clear SPI
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	// CS high
	SPI_CS_MEM_OUT |= SPI_CS_MEM_BIT;

	// TODO This will fail in many circumstances: We're returning a pointer to a local variable.
	return testArray;

} //end SPI write memory

void SPI_FRAM_Read_Image(const uint8_t *bufferAddress,
		uint16_t lengthData) {
	// CS low

	//uint8_t testArray[5807];
	SPI_CS_MEM_OUT &= ~SPI_CS_MEM_BIT;

	// send the op code
	for (uint16_t i = 0; i < 1; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = 0x03; //op code for reading memory (READ)
	}

	// wait for last byte to clear SPI
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	//send the 18-bit address
	for (uint16_t i = 0; i < 3; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = *bufferAddress++;
	}

	// clock out the 8-byte data
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	for (uint16_t i = 0; i < lengthData; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = 0xff;
		imageBuffer[i] = UCB1RXBUF;
	}

	// wait for last byte to clear SPI
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	// CS high
	SPI_CS_MEM_OUT |= SPI_CS_MEM_BIT;
} //end SPI write memory

