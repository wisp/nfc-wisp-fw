/******************************************************************************
 * crc_checker.h
 *
 * @date Jan 8, 2013
 * @author Derek Thrasher
 *****************************************************************************/

#ifndef CRC_CHECKER_H_
#define CRC_CHECKER_H_

#include <stdint.h>
unsigned char validate_crc(unsigned char* receive_array, unsigned char number_of_bytes);
unsigned int generate_crc(unsigned char* transmitCommand, unsigned int numberOfBits);
unsigned int hw_crc_ccitt(unsigned char* transmitCommand, unsigned int numberOfBits);
unsigned int hw_crc_validate(unsigned char* receiveCommand, unsigned int numberOfBits);

#endif /* CRC_CHECKER_H_ */
