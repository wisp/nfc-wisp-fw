/******************************************************************************
 * _14443_B_protocol.h
 *
 * @date   April 5, 2014
 * @author Eve(Yi Zhao), Sensor Systems Lab, UW
 * 		   Derek Thrasher
 ******************************************************************************/

#ifndef _14443_B_PROTOCOL_H_
#define _14443_B_PROTOCOL_H_

//globals
#include "_14443_B.h"
#include <stdint.h>

#define CID_Enable	BIT0
#define CID_Disable 0

extern unsigned int numberOfBitsTX;									//number of bits to transmit
extern uint8_t uid[UID_SIZE];
extern uint8_t _14443_buf_ptr;
extern uint8_t rx_buffer[CMD_BUF_SIZE];
extern uint8_t transmitCommand[CMD_BUF_SIZE];

//extern unsigned char imageBuffer[E_INK_SIZE];
//extern unsigned char imageBuffer[2];
extern unsigned char command_number;
extern unsigned int imageBytesReceived;

//function prototypes
void initialize_14443_B_protocol(void);
uint8_t _14443_B_protocol(uint8_t numb_bytes);
uint8_t uid_is_match(unsigned int);
unsigned int add_pupi(unsigned int start);
void toggle_block_number(void);
#endif /* PROTOCOL_H_ */
