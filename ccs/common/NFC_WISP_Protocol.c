/*
 * protocol.c
 *
 *  Created on: Mar 8, 2013
 *      Author: Derek Thrasher (UMass Amherst)
 */

#include "NFC_WISP_Protocol.h"

///////////////////////////////////////////////////////////////////////////////
// CONSTANTS
///////////////////////////////////////////////////////////////////////////////

// Enumerating indices into INF buffer
#define INF_CMD_INDEX 			0
#define INF_DATA_START_INDEX 	2

//Flag enumeration (R->T and T->R)
#define INF_FLAG_WRITE_SUCCESS 	0x01
#define INF_FLAG_WRITE_COMPLETE 0x02
//  0x04
//  0x08
//  0x10
//  0x20
//  0x40
//  0x80

////Command IDs for "transport layer"
//#define INF_CMD_ID_TX_COMPLETE	0x01
//#define INF_CMD_ID_READ_SINGLE_BLOCK 0x20
//#define INF_CMD_ID_WRITE_SINGLE_BLOCK 0x21
//#define INF_CMD_ID_WRITE_N_BLOCKS 0x24

#define BLOCK_SIZE 4
#define BLOCKS_PER_CHUNK 8
#define CHUNK_SIZE_BYTES (BLOCK_SIZE*BLOCKS_PER_CHUNK)

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
typedef struct {
	unsigned char raw[CHUNK_SIZE_BYTES];
} ArrayChunks_t;

ArrayChunks_t* imageBufferChunked;

unsigned char curRequestedChunk;

///////////////////////////////////////////////////////////////////////////////
//FUNCTION DEFINITIONS
///////////////////////////////////////////////////////////////////////////////

/**
 *
 */
void initialize_nfc_wisp_protocol() {

	curRequestedChunk = 0; // Initialize the image chunk counter
	imageBufferChunked = (ArrayChunks_t*)imageBuffer; // Map struct array over actual data buffer
}


/**
 * Interprets Higher Layer Response (INF) and decides how to fill in INF field of Response to ATTRIB
 *
 *
 * @param inf_received Represents the contents of the INF field of the incoming transaction
 * @param index Current index into the transmit command array, to which this function contributes
 *
 * @return The updated current index into the transmit command array after
 *
 */
unsigned char nfc_wisp_protocol(unsigned char * inf_received, unsigned char index) {

	// Status flags we'll send back to reader
	unsigned char tagResponseFlags = 0;

	// Extract chunk index from reader command
	unsigned char rxChunkIndex = inf_received[1];

	if(rxChunkIndex == curRequestedChunk) { // If the reader is sending the chunk we requested

		//!!!
		// Be careful, everything we do here should be done FAST! If it takes too long, reader will miss the tag's response.
		//!!!

		// Set up some pointers for a buffer copy speed run
		unsigned char* tmpDst = imageBufferChunked[rxChunkIndex].raw;
		unsigned char* finalDst = &(imageBufferChunked[rxChunkIndex].raw[CHUNK_SIZE_BYTES]); // Disregard out-of-range warning
		unsigned char* tmpSrc = &(inf_received[INF_DATA_START_INDEX]);

		// Copy data from INF field SUPER FAST (least dereferencing possible)!
		do {
			*(tmpDst++) = *(tmpSrc++);
		} while(tmpDst != finalDst);

		// Signal to reader that we've successfully copied the previous data
		tagResponseFlags |= INF_FLAG_WRITE_SUCCESS;

		// Since we successfully received that chunk, request the next one unless we're done with everything
		if(curRequestedChunk >= (E_INK_SIZE / CHUNK_SIZE_BYTES) - 1) {

			// Signal to reader that we've received all the chunks we think we need
			tagResponseFlags |= INF_FLAG_WRITE_COMPLETE;

			// Signal to application that we're done
			// TODO Hack alert! Encapsulate this module better
			imageTransferDoneFlag = 1;

			led_1_off(); // DEBUG

		} else {
			toggle_led_1(); // DEBUG
			curRequestedChunk++; // Request next chunk
		}

	} // end if

	// Tell the reader how things went this round
	transmitCommand[index++] = tagResponseFlags;

	// Send the latest requested address, which the reader should send to us next round
	transmitCommand[index++] = curRequestedChunk;

	return index;
}

