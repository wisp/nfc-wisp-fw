/******************************************************************************
 * @fun	 Demo for Temperature/Acceleration display&logging
 * @author Yi Zhao(Eve)
 * @date -09/29/2014
 *****************************************************************************/

//=============================================================================
//									Includes
//=============================================================================
#include "../common/globals.h"
#include "../common/e-paper.h"
#include "myE-paperApp.h"
#include "tempSense.h"
#include "img_data.h"
#include "../common/accel.h"
#include "myApp.h"
#include <string.h>

//=============================================================================
//									E-paper Globals
//=============================================================================
//@note the imageBuffer now is 1 byte since we need more buffer for sense_buf
//uint8_t imageBuffer[E_INK_SIZE]; // big array that stores all the display bits
uint8_t sense_buf[BUF_SIZE];
uint8_t* sense_buf_ptr;
uint8_t* sense_read_ptr;
uint8_t	 buf_full;
uint16_t senseCtr;
//=============================================================================
//								 Globals
//=============================================================================
volatile unsigned long 		RTC_ctr;//real time clcok counter
//uint8_t 		imageUpdateState;
volatile uint8_t			senseState;
volatile uint8_t			accelState;


//=============================================================================
//					User App & Functions
//=============================================================================

/**
*	@fun	initial real time clock for sampling temperature/accelerometer data
*	for temerature/accelerometer logger demo
*/
void initMyDemo1(void){

	//Initialize the real time clock for sample sensor at given intervals
	initRTC(RTC_3s);

	//Initial sensing state machine
	imageUpdateState = IMG_UPDATE;
	//imageUpdateState = IMG_HALT;

	//inital senseng state and senseng data buffer
	accelState = 0;
	senseCtr=0;
	buf_full=0;
	sense_buf_ptr = &sense_buf[0];
	sense_read_ptr = &sense_buf[0];
}


/**
*	@fun	initial imageUpdateState for image updates demo
*/
void initMyDemo2(void){
	//TA1CTL = 0;	//ACLK, upmode, 4khz clk
	//TA1CCTL0 &=~CCIE;
	//initRTC(RTC_15s);
	imageUpdateState = IMG_HALT;
}




/**
*	@fun	temerature/accelerator logger and display demo
*			will not save sensor data into the buffer
*/
void Demo1(void) {
	if((doNFC_state==NFC_Sleep)&&(imageUpdateState)){
		updateDisplay1((uint8_t*)templete);
		imageUpdateState = IMG_HALT;
	}
}

/**
*	@fun	image updates demo when response to cellphone
*			if it receive the whole image and update it
*	@note	NFC-WISP Reader App 1.0 will sending dummy data for power
*			after finishing image transfer.
*			If you build *BUILD_E_INK_2_0_BATT_FREE*
*			we add more lowPowerSleep during E-ink updating to
*			reive more energy from cell-phone
*/
void Demo2(void){
	if((doNFC_state==NFC_Sleep)&&(imageUpdateState==IMG_UPDATE)||(imageUpdateState==IMG_FORCE_UPDATE)){
		//memset(imageBuffer,0,E_INK_SIZE);//Comment this line to save power
#ifdef BATT_FREE
		lowPowerSleep(LPM_120ms);
#endif
		updateDisplay2((uint8_t*)imageBuffer);
#ifdef BATT_FREE
		lowPowerSleep(LPM_10ms);
#endif
		imageUpdateState = IMG_HALT;
	}
}

/**
*	@fun	temperature/acceleration data logger demo
*			save the sensor data to the buffer for reader to read
*	@note	need to change sense_buf size for your application
*/
void Demo3(void) {

		/****************Sensing handle routine 1 *********************************
		 *	@fun	log sensor result to buffer when reading sensor result
		 *	put 4 byte accelerometer data into app[0]-[3]
		 *	put 4 byte temperature data into uid[0]-[3]
		 *	if write is full than sense_read_ptr then stop write
		 *	@note when use this demo, must modify the command response to transmit
		 *	sensor result to the uid or app buffer
		 ***********************************************************************/

		if((doNFC_state==NFC_Sleep)&&(imageUpdateState)){

			//if sense_buf will be out of the range of queue
			if((sense_buf_ptr+8)>(&sense_buf[BUF_SIZE-1])){
				memset(sense_buf,0x00,&sense_buf[BUF_SIZE-1]-sense_buf_ptr+1);
				sense_buf_ptr=&sense_buf[0];
				buf_full++;
			}

			//if the queue is full, then do nothing
			if((sense_buf_ptr+8>sense_read_ptr)&&(buf_full>0)){
				  asm("NOP");
			}else if(imageUpdateState){
				 updateDisplay1((uint8_t*)templete);
				 ACCEL_singleSample(sense_buf_ptr);
				 sense_buf_ptr +=4;
				 senseCtr++;
			}
		}

}
