/*
 * protocol.h
 *
 *  Created on: June 5, 2013
 *      Author: Derek Thrasher
 */

#ifndef NFC_WISP_PROTOCOL_H_
#define NFC_WISP_PROTOCOL_H_

//globals
#include <stdint.h>
#include "msp430f5310.h"
#include "globals.h"
#include "crc_checker.h"

extern unsigned char _14443_buf_ptr;
//extern unsigned char rx_buffer[CMD_BUF_SIZE];
extern unsigned char transmitCommand[CMD_BUF_SIZE];

extern unsigned char imageBuffer[E_INK_SIZE];
extern unsigned char command_number;
extern unsigned int imageBytesReceived;

//function prototypes
void initialize_nfc_wisp_protocol(void);
uint8_t nfc_wisp_protocol(unsigned char * receiveCommand, unsigned char index);
#endif /* NFC_WISP_PROTOCOL_H_ */
