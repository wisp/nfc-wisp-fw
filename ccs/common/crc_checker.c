/*
 * crc_checker.c
 *
 *  Created on: Jan 8, 2013
 *      Author: Derek Thrasher (UMass Amherst)
 */

#include <stdint.h>
#include <msp430.h>
#include "crc_checker.h"

#define POLYNOMIAL 0x8408 // x^16 + x^12 + x^5 + 1
#define PRESET_VALUE 0xFFFF
#define CHECK_VALUE 0xF0B8
//#define NUMBER_OF_BYTES 4 // Example: 4 data bytes
//#define CALC_CRC 1
//#define CHECK_CRC 0



/*
//returns 1 is data is correct
uint8_t validate_crc(unsigned char* receive_array, uint8_t number_of_bytes){
	unsigned int current_crc_value = PRESET_VALUE;
	int i, j;
	for (i = 0; i < number_of_bytes; i++){
		current_crc_value = current_crc_value ^ ((unsigned int)receive_array[i]);
		for (j = 0; j < 8; j++){
			if (current_crc_value & 0x0001){
				current_crc_value = (current_crc_value >> 1) ^ POLYNOMIAL;
			}
			else{
				current_crc_value = (current_crc_value >> 1);
			}
		}
	}

		if (current_crc_value == CHECK_VALUE){
			//printf ("Checked CRC is ok (0x%04X)\n", current_crc_value);
			return (number_of_bytes - 0x02);
		}else{
			//printf ("Checked CRC is NOT ok (0x%04X)\n", current_crc_value);
			return 0x00;
		}
	}

unsigned int generate_crc(unsigned char* transmitCommand, uint16_t numberOfBits){
	unsigned int current_crc_value = PRESET_VALUE;
		int i, j;
		unsigned char number_of_bytes = numberOfBits/8;
		numberOfBits = numberOfBits + 16;
		for (i = 0; i < number_of_bytes; i++){
			current_crc_value = current_crc_value ^ ((unsigned int)transmitCommand[i]);
			for (j = 0; j < 8; j++){
				if (current_crc_value & 0x0001){
					current_crc_value = (current_crc_value >> 1) ^ POLYNOMIAL;
				}
				else{
					current_crc_value = (current_crc_value >> 1);
				}
			}
		}
		current_crc_value = ~current_crc_value;
			transmitCommand[number_of_bytes] = current_crc_value & 0xFF;
			transmitCommand[number_of_bytes + 1] = (current_crc_value & 0xFF00)>>8;
			return 1;
}
*/


/**
 * Use the hardware CRC-CCITT module of the MSP430 to generate a CRC as
 *  per ISO/IEC 13239 CRC16 specification used in ISO 15693. Generating
 *  polynomial is 0x8408, preset value is 0xFFFF.
 *
 *  CRC is appended to end of transmitCommand array.
 *
 * @author Aaron Parks
 */
unsigned int hw_crc_ccitt(unsigned char* transmitCommand, unsigned int number_of_bytes) {
	// Preset CRC result as per ISO/IEC 13239
	CRCINIRES = 0xFFFF;

	// Iterate over transmitCommand array
	int crc_i;
	for(crc_i=0; crc_i <number_of_bytes; crc_i++)
	{
		// Load next byte into LSB of data input register
		// TODO: Can this be done faster word-wise?
		CRCDI_L = transmitCommand[crc_i];
	}

	CRCINIRES = ~CRCINIRES; // Invert bits of result

	// Store result from bit-order-reversed version of CRCRES
	transmitCommand[number_of_bytes] = CRCRESR & 0x00FF;
	transmitCommand[number_of_bytes + 1] = __swap_bytes(CRCRESR) & 0x00FF;

	return 1;
}

/**
 * Use the hardware CRC-CCITT module of the MSP430 to validate a CRC as
 * generated per ISO/IEC 13239 CRC16 specification used in ISO 15693. Generating
 * polynomial is 0x8408, preset value is 0xFFFF.
 *
 * @author Derek Thrasher
 */

unsigned int hw_crc_validate(unsigned char* receiveCommand, unsigned int number_of_bytes) {

	// Preset CRC result as per ISO/IEC 13239
	CRCINIRES = 0xFFFF;
	if(number_of_bytes < 2)
		return 0;
	// Iterate over transmitCommand array
	//In essence we calculate the CRC and then will check it with the one appended to the receiveCommand
	unsigned int crc_i;
	for(crc_i = 0; crc_i < number_of_bytes - 2; crc_i++)
	{
		// Load next byte into LSB of data input register
		// TODO: Can this be done faster word-wise?
		CRCDI_L = receiveCommand[crc_i];
	}

	CRCINIRES = ~CRCINIRES; // Invert bits of result

	// Checks result from bit-order-reversed version of CRCRES to the received CRC (Last 2 bytes of receiveCommand)
	if((receiveCommand[number_of_bytes - 2] == (CRCRESR & 0x00FF)) && (receiveCommand[number_of_bytes - 1] == (__swap_bytes(CRCRESR) & 0x00FF)))
	{
		return (number_of_bytes - 2);
	}else
		return 0;

}
