/*
 * 1outof256.c
 *
 *  Created on: Mar 8, 2013
 *      Author: Jeremy Gummeson
 */

#include "1outof256.h"
#include "crc_checker.h"

//globals

//Where are we in the frame? Possibilities include: SOF, Frame Bytes, EOF
uint8_t frame_state;

//keep track of a pair of captures.  What is important is when one occurs relative to the other.
uint16_t capture_buffer[CAPTURE_BUFFER_SIZE];

//these indices keep track of pulse capture times stored in "capture_buffer"
uint8_t capture_buffer_write_index;
uint8_t capture_buffer_read_index ;

//time between the last two pulses we received. Used to figure out the first byte of a frame (flags)
uint32_t pulse_delta;
// sum of all the pulse deltas. Used to figure out subsequent bytes with a bit of fixed-point arithmetic
uint32_t sum_delta;

//which byte are we on?
uint8_t receive_index;

void initialize_1outof256()
{
	int i;

	for(i = 0; i < CAPTURE_BUFFER_SIZE; i++)
	capture_buffer[i] = 0;  //need to initialize the zero time case (this is why our initial indices are 1 greater

	pulse_delta = 0;
	sum_delta = 0;  //terrible, but necessary. Need room for fixed point

	capture_buffer_write_index = 0;  //start writing to the second position, since the first holds the 0 initialization value
	capture_buffer_read_index = 0;
	pending_captures = 0;

	num_frame_bytes = 0;
	frame_state = FINDING_DELIMITER;

	receive_index = 0;
	//configure timer capture pin
	TA0CTL |= 	TASSEL_2 |	//SMCLK as source
				MC_2 |		//CONTINUOUS MODE
				ID_3; 		//DIVIDE BY 8

	TA0CCTL2 |= CM_2	|	//falling edge capture
				CCIS_0	|	//capture input CCI1A
				SCS		|	//synchronous with clock
				CCIE	|	//enable interrupt capture
				CAP;		//enable capture mode
}

void process_bits()
{
	uint16_t t1;
	uint16_t t2;

	uint16_t byte_ticks;

	uint16_t temp;
	_bic_SR_register(GIE); //lock to protect indices

	t2 = capture_buffer[capture_buffer_read_index];

	if(capture_buffer_read_index == 0)
		t1 = capture_buffer[CAPTURE_BUFFER_SIZE - 1];

	else
		t1 = capture_buffer[capture_buffer_read_index - 1];

	if(capture_buffer_read_index == (CAPTURE_BUFFER_SIZE - 1))
		capture_buffer_read_index = 0;

	else
		capture_buffer_read_index++;

	pending_captures--;

	//normal case
	if(t2 >= t1)
		pulse_delta = t2 - t1;

	//overflow case -- check me on this...might have an off by 1 error (or worse...) (Approved -Derek)
	else if(t1 > t2)
		pulse_delta =  (0xffff - t1) + t2 + 1;

	_bis_SR_register(GIE); //unlock index protection
	switch(frame_state)
	{
		case FINDING_DELIMITER:

			if((pulse_delta >= SOF_LOW) && (pulse_delta <= SOF_HIGH)) //nominally 112 us
			{
				receive_index = 0;
				frame_state = PROCESSING_BYTES;
				toggle_debug();

			}

			break;

		//handle message frame bytes
		case PROCESSING_BYTES:

			//two pulses should never be this far apart. Find another delimiter.
			if(pulse_delta > TIMEOUT)
			{
				receive_index = 0;
				frame_state = FINDING_DELIMITER;
			}

			//handle message bytes
			else if(receive_index == 0)
			{
				receiveCommand[receive_index] = (pulse_delta - 8) >> 5;

				receive_index++;
				sum_delta = pulse_delta;
			}

			else
			{
				sum_delta = (sum_delta + pulse_delta);
				byte_ticks = sum_delta % TICKS_PER_BYTE;

				if(receive_index < (RECEIVE_BUF_SIZE - 1))
				{
<<<<<<< HEAD
					temp = ((byte_ticks) - 8) >> 4; //Divide by 16 for "half-slots". Minus 8 brings you to the middle of that half-slot in ticks.
=======
					temp = ((byte_ticks) - 5) >> 4; //Divide by 16 for "half-slots". Minus 8 brings you to the middle of that half-slot in ticks. //5 works ok for handshake
>>>>>>> Experimental_nfc_eink_demo

					//End of frame is always found in the 3rd "half-slot" or index 2.
					//The end of frame should also always be found after we have observed
					//5 bytes (3 inventory message bytes + 2 crc bytes)

					//if((temp == EOF_SLOT_POSITION) && ((receiveCommand[0] == FLAGS) || (receiveCommand[0] == 0x42)) && (receive_index >= (MIN_VALID_LENGTH - 1)))
					if((temp == EOF_SLOT_POSITION) && (receive_index >= (MIN_VALID_LENGTH - 1)))
					{
						//END OF FRAME DETECTION
						TA0R = 0x00;
						num_frame_bytes = hw_crc_validate(&receiveCommand[0], receive_index);
						toggle_debug();
						//Fault
						if(num_frame_bytes == 0)
						{
							numReceivedMessages++;
						}
						if(numReceivedMessages == 100)
						{
							asm(" nop");
						}

						//reset fault with inventory command
						if(receiveCommand[1] == 0x01)
							numReceivedMessages = 0;
						asm(" nop");
					}

					//If this isn't the end of frame, divide by 2 to convert from "half-slots"
					//to slots and add the byte value to the received buffer.
					else
					{
						receiveCommand[receive_index] = temp >> 1;
						receive_index++;
					}

				}

				//prevent buffer overrun.
				else
				{
					receive_index = 0;
					frame_state = FINDING_DELIMITER;
				}

			}
			break;
	}
	return;
}
