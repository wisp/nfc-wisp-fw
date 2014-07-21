/******************************************************************************
 * _14443_B_protocol.h
 *
 * @date June 5, 2013
 * @author Derek Thrasher
 ******************************************************************************/

#ifndef _14443_B_PROTOCOL_H_
#define _14443_B_PROTOCOL_H_

//globals
#include <stdint.h>
#include "msp430f5310.h"
#include "globals.h"
#include "crc_checker.h"
#include "_14443_B.h"
#include "NFC_WISP_PROTOCOL.h"


#define UID_SIZE 4


extern unsigned int numberOfBitsTX;									//number of bits to transmit
extern uint8_t uid[UID_SIZE];
extern uint8_t _14443_buf_ptr;
extern uint8_t rx_buffer[CMD_BUF_SIZE];
extern uint8_t transmitCommand[CMD_BUF_SIZE];

extern unsigned char imageBuffer[E_INK_SIZE];
extern unsigned char command_number;
extern unsigned int imageBytesReceived;

//function prototypes
void initialize_14443_B_protocol(void);
void _14443_B_protocol(uint8_t numb_bytes);
unsigned int uid_is_match(unsigned int);
unsigned int add_pupi(unsigned int start);
void toggle_block_number(void);
#endif /* PROTOCOL_H_ */
