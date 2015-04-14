/******************************************************************************
 * myNFC_Protocol.c
 *	@brief	    Higher Layer Application Response (INF) (to I-Block and \
 *				decides how to fill in INF field of Response to ATTRIB)
 *  @date: 		Aug 22, 2014
 *  @author: 	Yi Zhao (Eve)-Sensor System Lab, UW
 *  @note:		Only Implement response in ISO 14443-B protocol here
 *  @TODO:		Test and Implement _15693_1of256 protocol
  *****************************************************************************/

//=============================================================================
//									Includes
//=============================================================================
#include <string.h>
#include "myNFC_Protocol.h"
#include "../NFC_protocol/doNFC.h"
#include "../common/e-paper.h"

//=============================================================================
//									Defines
//=============================================================================
// Enumerating indices into INF buffer
#define INF_CMD_INDEX 			0
#define INF_DATA_START_INDEX 	2

//Flag enumeration (R->T and T->R)
#define INF_FLAG_WRITE_SUCCESS 	0x01
#define INF_FLAG_WRITE_COMPLETE 0x02
#define INF_FLAG_IMGE_UPDATE_COMPLETE 0x04

////Command IDs for "transport layer"
//#define INF_CMD_ID_TX_COMPLETE	0x01
//#define INF_CMD_ID_READ_SINGLE_BLOCK 0x20
//#define INF_CMD_ID_WRITE_SINGLE_BLOCK 0x21
//#define INF_CMD_ID_WRITE_N_BLOCKS 0x24


#define BLOCK_SIZE 4
#define BLOCKS_PER_CHUNK 15
#define CHUNK_SIZE_BYTES (BLOCK_SIZE*BLOCKS_PER_CHUNK)
#define MAX_CHUNK_SIZE ((E_INK_SIZE / CHUNK_SIZE_BYTES) - 1)

//=============================================================================
//									Local Variables
//=============================================================================
typedef struct {
	unsigned char raw[CHUNK_SIZE_BYTES];
} ArrayChunks_t;

static ArrayChunks_t* imageBufferChunked;

static unsigned char curRequestedChunk;
static unsigned char rxChunkIndex;
//DEBUG
//extern uint8_t debugBug[10];

//=============================================================================
//									Functions
//=============================================================================
/**
 * initialize custermized nfc_wisp_protocol response to I-Block
 */
void initialize_nfc_wisp_protocol() {
	//doNFC_state=0;
	curRequestedChunk = 0; 								 // Initialize the image chunk counter
	imageBufferChunked = (ArrayChunks_t*)imageBuffer;   // Map struct array over actual data buffer
	//memset(rx_buffer, 0x00, CMD_BUF_SIZE);
	doNFC_state = NFC_Start;
	imageUpdateState = IMG_HALT;										// Clear flag
	senseState = 0;
	rxChunkIndex=0;
}


/**
 * Interprets Higher Layer Response (INF) to I-Block and \
 *  decides how to fill in INF field of Response to ATTRIB
 *
 * @param inf_received Represents the contents of the INF field of the incoming transaction
 * @param index Current index into the transmit command array, to which this function contributes
 *
 * @return The updated current index into the transmit command array after
 *
 */
unsigned char nfc_wisp_protocol(unsigned char * inf_received, unsigned char index) {


	// Extract chunk index from reader command


	// Status flags we'll send back to reader
	unsigned char tagResponseFlags = 0;

	//Used to indicat finish of image update, if INF_FLAG_IMGE_UPDATE_COMPLETE is set
	tagResponseFlags=inf_received[0];

	rxChunkIndex = inf_received[1];
	//@DEBUG
	//debugBuf[0]=curRequestedChunk;
	if((rxChunkIndex == curRequestedChunk)&&(!(tagResponseFlags&INF_FLAG_IMGE_UPDATE_COMPLETE))) { // If the reader is sending the chunk we requested
	//if((rxChunkIndex == curRequestedChunk)&&(!(rxChunkIndex&INF_FLAG_IMGE_UPDATE_COMPLETE))) { // If the reader is sending the chunk we requested

		//!!!
		// Be careful, everything we do here should be done FAST! If it takes too long, reader will miss the tag's response.
		//!!!

		// Set up some pointers for a buffer copy speed run
		unsigned char* tmpDst = imageBufferChunked[rxChunkIndex].raw;
		unsigned char* finalDst = &(imageBufferChunked[rxChunkIndex].raw[CHUNK_SIZE_BYTES]); // Disregard out-of-range warning
		unsigned char* tmpSrc = &(inf_received[INF_DATA_START_INDEX]);

		//led_2_on();
		// Copy data from INF field SUPER FAST (least dereferencing possible)!
		do {
			*(tmpDst++) = *(tmpSrc++);
		} while(tmpDst != finalDst);

		// Signal to reader that we've successfully copied the previous data
		tagResponseFlags |= INF_FLAG_WRITE_SUCCESS;

		// Since we successfully received that chunk, request the next one unless we're done with everything
		//if(curRequestedChunk >= (E_INK_SIZE / CHUNK_SIZE_BYTES) - 1) {
		if(curRequestedChunk >= MAX_CHUNK_SIZE) {

			// Signal to reader that we've received all the chunks we think we need
			tagResponseFlags |= INF_FLAG_WRITE_COMPLETE;

			// Signal to application that we're done
			// TODO Hack alert! Encapsulate this module better
			imageUpdateState = IMG_FORCE_UPDATE;
			curRequestedChunk = 0;
			//led_2_on(); // DEBUG

		} else {
			//toggle_led_1(); 				// DEBUG
			curRequestedChunk++; 			// Request next chunk
			//debugBuf[9]=curRequestedChunk;
		}

	}

	// Tell the reader how things went this round
	transmitCommand[index++] = tagResponseFlags;

	// Send the latest requested address, which the reader should send to us next round
	transmitCommand[index++] = curRequestedChunk;

	return index;
}

