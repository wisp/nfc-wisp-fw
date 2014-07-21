/*
 * 1outof256.h
 *
 *  Created on: Mar 8, 2013
 *      Author: Jeremy
 */

#ifndef ONE_OUT_OF_TWO_FIFTY_SIX_H_
#define ONE_OUT_OF_TWO_FIFTY_SIX_H_

#include <msp430f5310.h>
#include "globals.h"

//receiver state machine constants
#define FINDING_DELIMITER 	0
#define PROCESSING_BYTES 	1

//static buffer sizes
#define RECEIVE_BUF_SIZE 40
#define NUM_COMMANDS 15
#define DATA_BUF_SIZE 64


//PROTOCOL CONSTANTS
//Protocol timeout. This is equal to 2 1outof256 byte times. If receiving a valid fr ame, the next byte pulse should always arrive before this timeout value.
//Longest valid time is 0x00 followed by 0xff, which is 1 slot shorter than 2 byte times.
#define TIMEOUT 16386 //more than two byte times worth of timer ticks (2*4.833 ms @ 13.56 MHz / 8).
#define SOF_WIDTH 112 // nominal number of timer ticks for the start of frame delimiter
#define SOF_DELTA 1	//will tolerate this many ticks deviation from nominal value

#define TICKS_PER_SLOT 32 //There are 32 (13.56/8)MHz clock ticks per bit period
#define TICKS_PER_BYTE (TICKS_PER_SLOT * 256)

#define SOF_LOW (SOF_WIDTH - SOF_DELTA)
#define SOF_HIGH (SOF_WIDTH + SOF_DELTA)

#define EOF_SLOT_POSITION 2 //splitting a byte into 512 slots, the EOF will always occupy the 3rd slot (index 2). This should never be true for 0x01 (4th slot)

//The galaxy nexus sends 0x26 as the first byte containing flags.
//There might be more definitions here for different readers
#define NEXUS_FLAGS 0x26

//Change this guy to a particular reader
#define FLAGS NEXUS_FLAGS

//minimum length of a 15693 message (including CRC)
#define MIN_VALID_LENGTH 5

//from main.c
extern uint8_t num_frame_bytes;
extern uint8_t pending_captures;
extern unsigned int numReceivedMessages;
extern unsigned char receiveCommand[RECEIVE_BUF_SIZE];			//received command

void initialize_1outof256(void);

//this function processes outstanding 1 out of 256 pulses.
void process_bits(void);


#endif /* 1OUTOF256_H_ */
