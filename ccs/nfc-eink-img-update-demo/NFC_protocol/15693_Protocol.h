/*
 * protocol.h
 *
 *  Created on: Mar 8, 2013
 *      Author: Jeremy
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

//globals
#include <stdint.h>
#include "msp430f5310.h"
#include "send_hdr.h"
#include "1outof256.h"
#include "crc_checker.h"
#include "../common/globals.h"


#ifdef _15693_1of256
	#define UID_SIZE 8
#else
	#define UID_SIZE 4 //size of PUPI
#endif

extern uint16_t numberOfBits;									//number of bits to transmit
extern unsigned char receiveCommand[RECEIVE_BUF_SIZE];			//received command
extern unsigned char transmitCommand[CMD_BUF_SIZE];
extern unsigned char uid[UID_SIZE];

extern uint8_t cat_2_7_bits[E_INK_SIZE];
extern uint8_t command_number;
extern uint16_t image_byte_counter;

//function prototypes
void _15693_protocol(void);
uint16_t uid_is_match(uint16_t);
void initialize_15693_protocol(void);

#endif /* PROTOCOL_H_ */
