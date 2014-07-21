/*
 * protocol.c
 *
 *  Created on: Mar 8, 2013
 *      Author: Jeremy Gummeson, Derek Thrasher (UMass Amherst)
 */

#include "_14443_B_protocol.h"

unsigned char read_command_number;
unsigned char CID;
unsigned int pcd_max_frame_size;
unsigned int block_number;

/**
 * Initializes ISO_14443_B Protocol. Initializes NFC WISP PUPI and several protocol state variables
*/
void initialize_14443_B_protocol() {
	uid[0] = 0x01;
	uid[1] = 0x23;
	uid[2] = 0x45;
	uid[3] = 0x67;

	CID = 0x00;
	read_command_number = 0;
	pcd_max_frame_size = 0;
	block_number = 1;
}

//used by protocol to see if the reader sent command is targeting us
unsigned int uid_is_match(unsigned int start) {
	unsigned char i;
	//start = the first byte position of uid in receiveCommand
	for (i = 0; i < 8; i++) {
		if (uid[i] != rx_buffer[i + start]) {
			//UIDs do NOT match
			return 0x0000;
		}
	}
	//UID Match
	return 0x0001;
}

/*Partial implementation of the 14443 B Protocol Logic*/
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
void _14443_B_protocol(unsigned char num_bytes) {
	//protocol interprets the received command and decides how to respond to the reader(PCD)

	unsigned char index = 0;//initialize index so it can be used in dynamic responses
	numberOfBitsTX = 0;

	//Begin

	/////////////////////////////////////////////
	if (rx_buffer[0] == 0x05) { // REQB

		//initialize on first byte
		block_number = 1;

		transmitCommand[index++] = 0x50;
		//fillPupi
		index = add_pupi(index);
		//appdata
		transmitCommand[index++] = 0x00;
		transmitCommand[index++] = 0x00;
		transmitCommand[index++] = 0x00;
		transmitCommand[index++] = 0x00;
		//protocol info
		transmitCommand[index++] = 0x00;
		//TODO: may need to set 0x80 to 0x81 saying that we are compliant to -4
		transmitCommand[index++] = 0x81;//0x80 = 256 max frame bytes... 0x01 supports 14443-4
		//TODO What impact would increasing FWI have in the following config byte?
		transmitCommand[index++] = 0x00; // CID (BIT1) supported and NAD (BIT0) NOT Supported

		//CRC will be appended later at end


		/////////////////////////////////////////////
	} else if (rx_buffer[0] == 0x1D) {
		//Id = 4 bytes
		//Make sure PUPI matches
		for (int i = 0; i < 4; i++) {
			if (uid[i] != rx_buffer[1 + i])
				return;
		}
		//param = 3 bytes
		//param 1 can be used to speed up communication
		//rx_buffer[5]
		//param 2 max fram size
		pcd_max_frame_size = rx_buffer[6] & 0x0F;
		//((rx_buffer[6] & 0xF0) == 0) 106kbit/s communication
		//param 3
		//rx_buffer[7];
		//CID 1 byte
		CID = rx_buffer[8] & 0x0F;
		//INF = variable size
		transmitCommand[index++] = pcd_max_frame_size << 4 | CID;

		//index = nfc_wisp_protocol(&rx_buffer[9], index);

		/////////////////////////////////////////////
	} else if ((rx_buffer[0] & 0xC0) == 0x00) //Iblock
			{
		toggle_block_number();
		if (rx_buffer[0] & BIT4)		//Chaining
		{
			//block number --BIT0
			//need to respond with R(ACK)
			transmitCommand[index++] = 0xA2 | (block_number & BIT0);
			//toggle_led_2();
		} else //No chaining
		{
			//Block number == BIT0
			transmitCommand[index++] = 0x02 | (block_number & BIT0);
			if (_14443_buf_ptr > 3)
				index = nfc_wisp_protocol(&rx_buffer[1], index);
		}
		/////////////////////////////////////////////
	} else if ((rx_buffer[0] & 0xE0) == 0xA0) //R Block
			{

		if ((rx_buffer[0] & BIT0)== block_number) {
			//re-transmit last block
		} else if (rx_buffer[0] & BIT4) //NAK
		{
			if ((rx_buffer[0] & BIT0)!= block_number) {
				transmitCommand[index++] = 0xA2 | (block_number & BIT0); //R(ACK)
				//toggle_led_2();
			}
		} else				//ACK
		{
			if ((rx_buffer[0] & BIT0)!= block_number) {
				toggle_block_number();
			}
			//SEND NEXT BLOCK / continue chaining
			//index = nfc_wisp_protocol(&rx_buffer[9], index);
			//toggle_led_2();
		}
		/////////////////////////////////////////////
	} else if (rx_buffer[0] == 0xC2) //SBlock
			{
		//Deselect S-Block
		//Not responding to 0xC2 properly. this is when we are not -4 compliant
		//TODO need to figure out how to respond to a S(DESELECT)
		transmitCommand[index++] = 0xC2;
		//toggle_led_2();
		//index = nfc_wisp_protocol(&rx_buffer[9], index);

		/////////////////////////////////////////////
	} else { // Unknown command
		//transmitCommand[index++] = CID ;
		//int i = 0;
		//index = nfc_wisp_protocol(&rx_buffer[9], index);

	} // end if

	// Generate and append CRC
	hw_crc_ccitt(&transmitCommand[0], index);

	// Compute length of transmitCommand[]
	numberOfBitsTX = (index * 8) + 16;

	return;
}

void toggle_block_number() {
	if (block_number) {
		block_number = 0;	//set low
	} else {
		block_number = 1;		//set high
	}
}

unsigned int add_pupi(unsigned int start) {
	unsigned int i;
	for (i = 0; i < UID_SIZE; i++) {
		transmitCommand[start++] = uid[i];
	}
	return start;
}
