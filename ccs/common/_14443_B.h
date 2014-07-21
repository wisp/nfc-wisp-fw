/******************************************************************************
 * _14443_B.h
 *
 * @date May 31, 2013
 * @author Derek Thrasher
 *****************************************************************************/

#ifndef _B_H
#define _B_H

#include "globals.h"

//for 6.78Mhz clock
// TODO Get rid of ifdefs if possible
#ifdef _15693_1of256
#define TIME_OUT 90
#else
#define TIME_OUT 196
#endif

//extern uint8_t _14443_buf_ptr = 0;
extern unsigned short _14443_delimiter_found;
extern uint8_t rx_buffer[CMD_BUF_SIZE];




void initialize_14443(void);
unsigned short find_delimiter(void);

#endif /* 14443_B_H_ */
