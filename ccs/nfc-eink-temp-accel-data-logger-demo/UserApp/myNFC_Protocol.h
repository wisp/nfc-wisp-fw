/******************************************************************************
 * myNFC_protocol.h
 *	@brief:		Higher Layer Application Response (INF) (to I-Block and \
 *				decides how to fill in INF field of Response to ATTRIB)
 *  @date: 		Aug 22, 2014
 *  @author: 	Yi Zhao (Eve)-Sensor System Lab, UW
 *  @note:		Only Implement response in ISO 14443-B protocol here
 *  @TODO:		Test and Implement _15693_1of256 protocol
  ******************************************************************************/

#ifndef MYNFC_PROTOCOL_H_
#define MYNFC_WISP_PROTOCOL_H_

//globals
#include <stdint.h>
#include "msp430f5310.h"
#include "../common/globals.h"
#include "../NFC_protocol/crc_checker.h"

extern unsigned char _14443_buf_ptr;
//extern unsigned char rx_buffer[CMD_BUF_SIZE];
//extern unsigned char transmitCommand[CMD_BUF_SIZE];
//extern unsigned char imageBuffer[E_INK_SIZE];
//extern unsigned char imageBuffer[2];
extern unsigned char command_number;
extern unsigned int imageBytesReceived;

//function prototypes
void initialize_nfc_wisp_protocol(void);
uint8_t nfc_wisp_protocol(unsigned char * receiveCommand, unsigned char index);
#endif /* NFC_WISP_PROTOCOL_H_ */
