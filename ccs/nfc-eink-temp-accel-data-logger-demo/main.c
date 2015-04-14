/******************************************************************************
 * @file	main.c
 *
 * @brief	Main structure for NFC WISP application
 *
 * @details The demo is designed to configure NFC-WISP as ISO-14443B tag,
 * 			and update the E-ink display.
 * 			User application can be modified in UserApp/myApp.c
 * 			User E-ink App can be modified in UserApp/myE-apperApp.c
 * 			NFC protocol in NFC_protocol can be modified if needed.
 *
 * @note 	NFC WISP Tag can be read by Nexus S, Nexus 5, Samsung Galaxy S4
 * 			But the E-ink image updates only works on Nexus S (API 19).
 * 			We do not use FRAM in this version
 *
 * @note	Must set debuger voltage to be 2.5V(2500) when debuging the board,
 * 			otherwise, some periferial may not work correctly!
 * @author	Eve (Yi Zhao) - Sensor Systems Lab (UW)
 * @date	09/29/2014
 * @TODO    Impelment ISO_15693
 *****************************************************************************/

//=============================================================================
//									Includes
//=============================================================================
// we will use ISO 14443B
//#define _15693_1of256							//havenet implement it yet

#include "common/globals.h"
#include "NFC_protocol/doNFC.h"
#include "common/e-paper.h"
#include "common/fram_memory.h"
#include "common/accel.h"
#include "UserApp/myApp.h"

/**
 * Main loop for application.
 */
int16_t main(void) {
	// Initialize MSP430 and peripherals
	//already done in _system_pre_init.c;

	// Initialize the RFID subsystem
  	initRFID();

	// Initialize the EPD subsystem (optional)
	initEPD();


	// Set up SPI and baud rate the ACCEL SPI clk are recommand as 1MHz~ 5 MHz
	SPI_initialize();

	//Put FRAM memory module into sleep, must wait at least for 1ms after whole chip power-up
	initFRAM();

	// Initialize the Accelerometer must wait at least for 5ms after whole chip power-up
	initACCEL();
   // ACCEL_standby();		//Put ACCEL into sleep mode if not use

	//User App initial
	initMyDemo1();			//temperature/acceleration dataLoggor demo
	//initMyDemo2();			//E-ink display update demo

	while (1) {
		#ifdef _15693_1of256
				doRFID_15693();
		#else

			/******************NFC and custermized App handle routine********************
			 * 	doNFC(), system will goto sleep if no NFC signal
			 * 			 and will wakeup if there is interrupt
			 * **************************************************************************/
			doNFC();

			/******************Sensing handle routine************************************/

			/**
			 * Temerature and Movement logger demo,
			 * @note temperature offset in different
			 *		 PCB is diffenrent, need to be manually adjust in tempSense.c
			 * 		 line #56 IntDegC = ((temp*93)>>4)-2650; //2860 is the offset for board 2
			 */
			Demo1();


			/**
			 * Updating E-ink screen with/withou battery using Nexus S and Andriod readerApp1.0 app
			 */
			//Demo2();
		#endif
	}

}
