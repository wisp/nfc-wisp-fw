/*
 * protocol.c
 *
 *  Created on: Mar 8, 2013
 *      Author: Jeremy Gummeson, Derek Thrasher
 */

#include "15693_Protocol.h"
//#include "globals.h"



//Derek Thrasher
//mask_length and AFI not currently being used for anything
unsigned char *mask_length;
unsigned char *AFI;
unsigned char mask_value[8];

//Can do away with both of these and just assign transmitCommand a value straight from receiveCommand...
unsigned char response_flags;
unsigned char DSFID; //Data Storage Format ID

uint8_t read_command_number;

uint16_t VICC_state;
uint16_t process_request_flag;

void initialize_15693_protocol()
{
	uid[0] = 0xFE;
	uid[1] = 0xDC;
	uid[2] = 0xBA;
	uid[3] = 0x98;
	uid[4] = 0x76;
	uid[5] = 0x54;
	uid[6] = 0x32;
	uid[7] = 0x10;

	VICC_state = 0x0001;
	process_request_flag = 0;
	read_command_number = 0;
}

//used by protocol to see if the reader sent command is targeting us
uint16_t uid_is_match(uint16_t start){
	uint8_t i;
	//start = the first byte position of uid in receiveCommand
	for(i=0 ; i<8 ; i++){
		if(uid[i] != receiveCommand[i+start]){
			//UIDs do NOT match
			return 0x0000;
		}
	}
	//UID Match
	return 0x0001;
}

/*Partial implementation of the 15693 Protocol Logic*/
/**
bitCount is the number of valid bits in the receiveCommand array [guaranteed to be >0]
user should update the following:
	transmitCommand[]	<< data to be sent, index 0 is least significant word
	numberOfBits		<< number of bits out of the command array to send (when set to 0:
							cyclesToDelay does not need to be set)
	cyclesToDelay		<< number of cycles to delay after the last rising edge of received
							data
NOTE: as numberOfBits increases, the amount of time available to the user decreases
		worst case: ~25 cycles/bit, best case: ~15 cycles/bit (includes all edges beyond
		beyond the start bit)
NOTE: if the user's computation requires more time than the requested delay, data will
		not be sent as the reader may or may not acknowledge the command
*/
void _15693_protocol(){
	// The PICC can be in 4 states
	//	Power-Off	/	Ready	/	Quiet	/	Selected
	//State= 0		/	1		/	2		/	3

	uint8_t i;

	uint8_t index;

	index = 0;//initialize index so it can be used in dynamic responses
	numberOfBits = 0;

	if(receiveCommand[1]==0x01)
	{
			response_flags = 0x00; //no error, no protocol format extension
			DSFID = 0x00; //VICC programming not supported
			transmitCommand[index++] = response_flags;
			transmitCommand[index++] = DSFID;
			for(i=0 ; i<8 ; i++){
				transmitCommand[index++] = uid[i];
			}
			numberOfBits = index*8;
	}


	//single block write
	else if(receiveCommand[1] == 0x21)
	{
		cat_2_7_bits[image_byte_counter++] = receiveCommand[3];
		cat_2_7_bits[image_byte_counter++] = receiveCommand[4];
		cat_2_7_bits[image_byte_counter++] = receiveCommand[5];
		cat_2_7_bits[image_byte_counter++] = receiveCommand[6];
		response_flags = 0x00; //no error, no protocol format extension
		DSFID = 0x00; //VICC programming not supported
		transmitCommand[index++] = response_flags;
		transmitCommand[index++] = DSFID;
		numberOfBits = index*8;
		toggle_debug();
	}

	else if(receiveCommand[1] == 0x24)
	{
		int i;
		int j = (receiveCommand[2]+1);
		j=j*4;
		for(i = 0 ; i < j ; i++)
		{
			cat_2_7_bits[image_byte_counter++] = receiveCommand[3+i];
		}
		/*
		cat_2_7_bits[image_byte_counter++] = receiveCommand[3];
		cat_2_7_bits[image_byte_counter++] = receiveCommand[4];
		cat_2_7_bits[image_byte_counter++] = receiveCommand[5];
		cat_2_7_bits[image_byte_counter++] = receiveCommand[6];
		cat_2_7_bits[image_byte_counter++] = receiveCommand[7];
		cat_2_7_bits[image_byte_counter++] = receiveCommand[8];
		cat_2_7_bits[image_byte_counter++] = receiveCommand[9];
		cat_2_7_bits[image_byte_counter++] = receiveCommand[10];
		*/
		response_flags = 0x00; //no error, no protocol format extension
		DSFID = 0x00; //VICC programming not supported
		transmitCommand[index++] = response_flags;
		transmitCommand[index++] = DSFID;
		numberOfBits = index*8;
	}


	hw_crc_ccitt(&transmitCommand[0], index);
	numberOfBits = numberOfBits + 16;
	/*
	if(numberOfBits){
		if(receiveCommand[1] == 0x01)
		{
			//generate_crc(transmitCommand, numberOfBits);
			transmitCommand[10] = 0x21; //precomputed CRC Byte #2
			transmitCommand[11] = 0xBE; //precomputed CRC Byte #2
			numberOfBits = numberOfBits +16;
		}

		else if((receiveCommand[1] == 0x21) || (receiveCommand[1] == 0x24))
		{
			transmitCommand[2] = 0x47; //precomputed CRC Byte #2
			transmitCommand[3] = 0x0F; //precomputed CRC Byte #2
			numberOfBits = numberOfBits +16;
			//numberOfBits = 0;
		}
	}
	 */
	return;
}
