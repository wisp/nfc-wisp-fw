/*
 * doNFC.h
 *
 *  Created on: Aug 22, 2014
 *      Author: evezhao
 */

#ifndef DONFC_H_
#define DONFC_H_

//=======================================================================================
//									Includes
//=======================================================================================
//#include <msp430.h>
#include "../common/globals.h"
#include "../UserApp/myApp.h"
// TODO Get rid of ifdefs if possible
#ifdef _15693_1of256
#include "1outof256.h"
#include "send_hdr.h"
#include "15693_Protocol.h"
#else
#include "_14443_B_protocol.h"
//#include "NFC_WISP_Protocol.h"
//#include "send_bpsk.h"
//#include "crc_checker.h"
//include "_14443_B.h"
#endif

//=============================================================================
//									NFC Globals
//=============================================================================

//This stuff is totally global in nature. I'm not sure it belongs here. But here it is.
//After the CRC is verified, this gets set to the number of bytes we received.
extern uint8_t 				num_frame_bytes;
extern uint8_t	volatile 	doNFC_state;
// TODO Get rid of ifdefs if possible
#ifdef _15693_1of256

extern uint8_t transmitCommand[CMD_BUF_SIZE];

extern uint8_t uid[UID_SIZE]; /**< Byte array that stores the UID. Initialized by _15693_protocol.c. */
extern unsigned int numReceivedMessages; /**< Number of messages received from a reader. Used to track image transfer progress. */
extern uint8_t pending_captures; /**< Tracks pending captures for ISO_15693. This is the number of outstanding pulses to be processed. */
extern unsigned int numberOfBitsTX; /**< Number of bits to transmit. Transmit function uses this to know how much of the buffer to send. */
//unsigned int bit_count; //used which checking crc

//externs
extern unsigned int capture_buffer[CAPTURE_BUFFER_SIZE];
extern uint8_t capture_buffer_write_index;

extern uint8_t receiveCommand[RECEIVE_BUF_SIZE];//received command
#else // otherwise assume 14443_B
extern uint8_t _14443_buf_ptr; /**< 14443_rx buffer pointer. Points to the current buffer index of received buffer. */
extern uint8_t rx_buffer[CMD_BUF_SIZE]; /**< 14443_B RX message buffer. Stores CMD_BUF_SIZE bytes. */
extern uint8_t transmitCommand[CMD_BUF_SIZE]; /**< 14443_B TX message buffer. Stores CMD_BUF_SIZE bytes. */
//extern uint8_t doNFC_state;
extern uint8_t uid[UID_SIZE]; /**< Byte array that stores the PUPI. Initialized by _14443_B_protocol.c. */

extern unsigned int numberOfBitsTX;	/**< Number of bits to transmit. Transmit function uses this to know how much of the buffer to send. */
#endif /* _15693_1of256 */
//=============================================================================
//							Macros
//=============================================================================
// Setup interrupt on edges
// 1 Falling edge wakeup, but it is actually rising edge for RX_WAKE_UP pin
// 2 Enable interrupt
#define enable_Rx_Wakeup() \
	RX_WAKEUP_EN_OUT |= RX_WAKEUP_EN_BIT; \
	RX_WAKEUP_EN_DIR |= RX_WAKEUP_EN_BIT; \
	lowPowerSleep(4); \
	RX_WAKEUP_IES &= ~RX_WAKEUP_BIT; \
	RX_WAKEUP_DIR &=~ RX_WAKEUP_BIT; \
	RX_WAKEUP_IFG = 0; \
	RX_WAKEUP_IE = RX_WAKEUP_BIT;

// Disable Rx Wakeup and may start RX
#define disable_Rx_Wakeup() \
	RX_WAKEUP_IE &= ~RX_WAKEUP_BIT; \
	RX_WAKEUP_EN_DIR &= ~RX_WAKEUP_EN_BIT; \
	RX_WAKEUP_IFG &= ~(RX_WAKEUP_BIT);

#define	Sleep_TimeOut	2000//2000*0.25ms = 0.5s
#define	tempSense_TimeOut	0xF000 //1s is 0x1000 15s is 0xF000
//==============================================================================
//							Functions
//==============================================================================
void initRFID(void);
//void doRFID_15693(void);
//void doRFID_14443B(void);
//void sleep_until_edges(void);
//void sleep_until_read(void);
void doNFC(void);
#endif /* DONFC_H_ */
