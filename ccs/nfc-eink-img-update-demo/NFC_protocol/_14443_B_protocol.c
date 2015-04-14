/*
 * protocol.c
 *
 *  @date: 	 Mar 8, 2013
 *  @author  Eve(Yi Zhao), Sensor Systems Lab,  UW
 *  		 Jeremy Gummeson, Derek Thrasher (UMass Amherst)
 *
 */

#include "_14443_B_protocol.h"

#include "msp430f5310.h"
#include "../common/globals.h"
#include "crc_checker.h"
#include "../UserApp/myNFC_Protocol.h"

unsigned char read_command_number;
unsigned char CID;
unsigned int pcd_max_frame_size;
unsigned char lastTxIndex;
static unsigned int block_number;

/**
 * Initializes ISO_14443_B Protocol. Initializes NFC WISP PUPI and several protocol state variables
*/
void initialize_14443_B_protocol() {
    uid[0] = 0x23;
    uid[1] = 0x45;
    uid[2] = 0x45;
    uid[3] = 0x67;

    CID = 0x40;
    read_command_number = 0;
    pcd_max_frame_size = 0;		//TODO: should we change it to be bigger size?
    block_number = 1;
    lastTxIndex=0;
}

//used by protocol to see if the reader sent command is targeting us
uint8_t uid_is_match(unsigned int start) {
    unsigned char i;
    //start = the first byte position of uid in receiveCommand
    for (i = 0; i<UID_SIZE; i++) {
        if (uid[i] != rx_buffer[i + start]) {
            //UIDs do NOT match
            return FALSE;
        }
    }
    //UID Match
    return TRUE;
}

/*Partial implementation of the 14443 B Protocol Logic*/
/**
 * response to REQB/ATTRIB/DESELECT S-BLOCK/R-BLOCK/I-BOCK/
 bitCount is the number of valid bits in the receiveCommand array [guaranteed to be >0]
 user should update the following:
 transmitCommand[]  << data to be sent, index 0 is least significant word
 numberOfBits       << number of bits out of the command array to send (when set to 0:
 cyclesToDelay does not need to be set)
 cyclesToDelay      << number of cycles to delay after the last rising edge of received
 data
 NOTE: as numberOfBits increases, the amount of time available to the user decreases
 worst case: ~25 cycles/bit, best case: ~15 cycles/bit (includes all edges beyond
 beyond the start bit)
 NOTE: if the user's computation requires more time than the requested delay, data will
 not be sent as the reader may or may not acknowledge the command
 Note: The broadcome reader chip will send REQB,ATTRIB, DESELECT
 */
uint8_t _14443_B_protocol(unsigned char num_bytes) {
    //protocol interprets the received command and decides how to respond to the reader(PCD)

    unsigned char index = 0;//initialize index so it can be used in dynamic responses
    numberOfBitsTX = 0;
    //toggle_led_2();
    //Begin

    /////////////////////////////////////////////
    // REQB or WUPB or Slot-MARKER
    /////////////////////////////////////////////
    if ((rx_buffer[0]&0x0F)== 0x05) {
    //if ((rx_buffer[0]== 0x05)) { // REQB
        //led_2_on();
        //initialize on first byte
        block_number = 1;
        //@modify modify uid =uid+app data only for RFID conference
        //TODO change it back later
        transmitCommand[index++] = 0x50;
        //fillPupi
        index = add_pupi(index);
        #if !defined(TempDemo)
            transmitCommand[index++] = 0x00;
            transmitCommand[index++] = 0x00;
            transmitCommand[index++] = 0x00;
            transmitCommand[index++] = 0x00;
        #endif

        //protocol 1
        transmitCommand[index++] = 0x00;
        //TODO: may need to set 0x80 to 0x81 saying that we are compliant to -4
        //protocol 2
        transmitCommand[index++] = 0x81;//0x80 = 256 max frame bytes... 0x01 supports 14443-4
        //TODO What impact would increasing FWI have in the following config byte?
        //Protocol 3 FWI/Coding options
        transmitCommand[index++] = CID_Enable|0x40; // CID (BIT1) supported and NAD (BIT0) NOT Supported
        //transmitCommand[index++] = 0x01; // CID (BIT1) supported and NAD (BIT0) NOT Supported
        //CRC will be appended later at end

	/////////////////////////////////////////////
	//ATTRIB Command
	/////////////////////////////////////////////
    }else if(rx_buffer[0] == 0x1D){
        //Id = 4 bytes
        //Make sure PUPI matches
        uint8_t i;
        for (i = 0; i < UID_SIZE; i++) {
            if (uid[i] != rx_buffer[1 + i])
                return FALSE;
        }
        //param = 3 bytes
        //param 1 can be used to speed up communication
        //rx_buffer[5]
        //param 2 max fram size

        pcd_max_frame_size = rx_buffer[6] & 0x0F;
        //((rx_buffer[6] & 0xF0) == 0) 106kbit/s communication
        //param 3
        //rx_buffer[7];
        //CID 1 byte
        CID = rx_buffer[8] & 0x0F;
        //INF = variable size
        transmitCommand[index++] = pcd_max_frame_size << 4 | CID;
        //transmitCommand[index++] = CID;

        //index = nfc_wisp_protocol(&rx_buffer[9], index);

	/////////////////////////////////////////////
	//I block
    //Note: most of the phone will send 16(include CRC) I block for NDEF first
    //Data is 0x02,0x00,0xA4,0x04,0x00,0x07,0xD2,0x76,
    //        0x00,0x00,0x85,0x01,  0x01,0x00,0xB7,0xD4,0xF7
    // 0xA4 is the header of NDEF
	////////////////////////////////////////////
    }else if((rx_buffer[0] & 0xE2) == 0x02){//Iblock
        //toggle_block_number();
    	block_number ^= BIT0;
        if (rx_buffer[0] & BIT4)        //Chaining
        {
            //block number --BIT0
            //need to respond with R(ACK)
        	//@eve_change, should be the same block number of I block
			//transmitCommand[index++] = 0xA2 | (block_number & BIT0);
        	transmitCommand[index++] = 0xA2 | (rx_buffer[0]&0x09);
            //toggle_led_2();
        //No chaining(NFC app goes here)
        //Should response an I block with chainning bit not set
        }else{

            //Block number == BIT0
            transmitCommand[index++] = rx_buffer[0];

            //if it is not NDEF check
            if (_14443_buf_ptr > 3 && (rx_buffer[2]!=0xA4)){  //in NDEF Check rx_buffer{2}=0xA4
            //if ((3<_14443_buf_ptr) && (_14443_buf_ptr<16)){
                index = nfc_wisp_protocol(&rx_buffer[1], index);
            }
        }
    /////////////////////////////////////////////
    //SBlock
    //Should response S-Block command after receiving S-Block
    /////////////////////////////////////////////
    }else if((rx_buffer[0]&0xC7)==0xC2){ //SBlock
        //Not responding to 0xC2 properly. this is when we are not -4 compliant
        //TODO need to figure out how to respond to a S(DESELECT)
    	//Recieve DESELCT then reply DESELECT
        transmitCommand[index++] =rx_buffer[0];
        //toggle_led_2();
        //index = nfc_wisp_protocol(&rx_buffer[9], index);
    /////////////////////////////////////////////
    // R Block
    // rule:   When an R(ACK) block with a block number not equal to the current PICC’s block number is received, the
     //   	PICC toggles its block number before sending a block
    /////////////////////////////////////////////
    }else if((rx_buffer[0] & 0xE6) == 0xA2){ //R Block{
    	//if it is ACK
    	if (!(rx_buffer[0]&BIT4)){
    		//if block number is not equal to current block number
    		//then toggle block, if it is in chainning, then continue chain
    		if((rx_buffer[0] & BIT0)!= block_number){
    			block_number ^= BIT0;
    			//TODO: if it is in chainning, then chainning is continued.
    			//DEBUG test retransmit lastTxIndex, should be commented out
    			//index=lastTxIndex;
    		}else{
    			//TODO: resend last block
				//asm("NOP");
    			index=lastTxIndex;
    		}
    	//shoud add funtion to response to chain later

    	//if it is R(NAK) 0xB3
    	}else if(rx_buffer[0] & BIT4){
    		if((rx_buffer[0] & BIT0)!= block_number){
    			//if block number is not equal then send R(ACK)
    			block_number ^= BIT0;
    			//transmitCommand[index++] = 0xA2 + (rx_buffer[0] & BIT0); //R(ACK)
    			//DEBUG: what happend if already response R(NAK) with last block
    			//index=lastTxIndex;
    		}else{
    			//TODO: implement resend the last block
    			//If block number is equal then resend last block
    			index=lastTxIndex;
    		}
    	}

    //HLTB Command
    }else if((rx_buffer[0]== 0x50)){
    	transmitCommand[index++] = 0x00;
    }else{ // Unknown command

        //transmitCommand[index++] = CID ;
        //int i = 0;
        //index = nfc_wisp_protocol(&rx_buffer[9], index);
        return FALSE;
    } // end if

    // Generate and append CRC
    hw_crc_ccitt(&transmitCommand[0], index);
    lastTxIndex = index;//Save the index into global buffer
    // Compute length of transmitCommand[]
    numberOfBitsTX = (index * 8) + 16;

    return TRUE;
}


/**********************************************************************
 * @note For TempDemo we use uid+app to log data
 *********************************************************************/
unsigned int add_pupi(unsigned int start) {
    unsigned int i;
    for (i = 0; i < UID_SIZE; i++) {
        #ifdef TempDemo
            transmitCommand[start++] = *sense_read_ptr++;

            //if the quence is empty transmit 00
            if((sense_read_ptr>=sense_buf_ptr)&&(buf_full<1)){
                //*sense_read_ptr = 0x00;
                sense_read_ptr=&sense_buf[0];
                senseCtr=0;
            }
            if(sense_read_ptr>&(sense_buf[BUF_SIZE-1])){
                buf_full--;
                sense_read_ptr=&sense_buf[0];
                senseCtr=0;
            }
        #else
            transmitCommand[start++] = uid[i];
        #endif


    }
    return start;
}
/*
unsigned int add_appData(unsigned int start) {
    unsigned int i;
    for (i = 0; i < APP_SIZE; i++) {
        transmitCommand[start++] = app[i];
    }
    return start;
}
*/
