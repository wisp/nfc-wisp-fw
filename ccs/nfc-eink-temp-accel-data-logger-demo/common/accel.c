/**
 * Driver for the ADXL362 accelerometer
 *
 * @author Aaron Parks,Eve(Yi Zhao), Sensor Systems Lab, UW
 * @date Aug 2013
 */

//=============================================================================
//									Includes
//=============================================================================
#include "accel.h"
#include "accel_registers.h"
//#include "globals.h"
#include "timer.h"
#include "spi.h"

//=============================================================================
//									Globals
//=============================================================================
// TODO Translate these to a better format once SPI driver is fleshed out. Make them CONST.
//uint8_t const ADXL_RESET[] = {ADXL_CMD_WRITE_REG,ADXL_REG_SOFT_RESET,0x00};
//uint8_t const ADXL_READ_PARTID[] = {ADXL_CMD_READ_REG,ADXL_REG_PARTID,0x00};
//uint8_t const ADXL_READ_DEVID[] = {ADXL_CMD_READ_REG,ADXL_REG_DEVID_AD,0x00};
uint8_t const ADXL_READ_STATUS[] = {ADXL_CMD_READ_REG,ADXL_REG_STATUS,0x00};
uint8_t const ADXL_READ_XYZ_8BIT[] = {ADXL_CMD_READ_REG,ADXL_REG_XDATA,0x00,0x00,0x00};
uint8_t const ADXL_READ_XYZ_STATUS_8BIT[] = {ADXL_CMD_READ_REG,ADXL_REG_XDATA,0x00,0x00,0x00,0x00};
//uint8_t const ADXL_READ_XYZ_16BIT[] = {ADXL_CMD_READ_REG,ADXL_REG_XDATA_L,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t const ADXL_CONFIG_MEAS[] = {ADXL_CMD_WRITE_REG,ADXL_REG_POWER_CTL,0x04}; // Put the ADXL into measurement mode,autosleep(0x02),wakeup(0x04)
//uint8_t const ADXL_CONFIG_MEAS[] = {ADXL_CMD_WRITE_REG,ADXL_REG_POWER_CTL,0x0E}; // Put the ADXL into measurement mode,autosleep,wakeup
//uint8_t const ADXL_CONFIG_DETECT[] = {ADXL_CMD_WRITE_REG,ADXL_REG_THRESH_ACT_L,0x90,0x00,0x02, 0x70,0x00,0x6,0x00,0x3F,0x00,0x80, 0x00,0x00,0x13,0x02};
//uint8_t const ADXL_CONFIG_READ[] = {ADXL_CMD_READ_REG,ADXL_REG_THRESH_ACT_L,   0x03,0x00,0x00,	0x50,0x00,0x2E,0x20,  0x3F,0x00,0x80,	 0x00,0x00,0x13,WAKE_UP};
//uint8_t const ADXL_CONFIG_DETECT[] = {ADXL_CMD_WRITE_REG,ADXL_REG_THRESH_ACT_L,0x50,0x00,0x04, 0x15,0x00,0x10,0x00,  0x3F,0x00,0x80,   0x00,0x00,0x13,0x0E};//paper setting
uint8_t const ADXL_CONFIG_DETECT[] = {ADXL_CMD_WRITE_REG,ADXL_REG_THRESH_ACT_L,0x50,0x00,0x04, 0x15,0x00,0x05,0x00,  0x3F,0x00,0x80,   0x00,0x00,0x13,0x0E};

/**
 * @note
 * Activity threshold(g)=code(0x30)/1024 = 0.04 g
* Activity detection time(s)=code(0x03/100HZ = 0.03s
* test Inactivity threshold(g)=code(0x28)/1024 = 0.039 g
* Inactivity threshold(g)=code(0x15)/1024 = 0.02 g
* Inactivity times=code(0xFF)/100 = 2.55s
* Inactivity times=code(0x80)/100 = 1.26s
* Loop mode 0x3F
* max data rate 50Hz
* when wakeup=1 set interrupt
* Wakeup mode and sleep
*/

//if just for log data
//uint8_t const ADXL_CONFIG_DETECT[] = {ADXL_CMD_WRITE_REG,ADXL_REG_THRESH_ACT_L,0xA0,0x00,0x0C, 0x90,0x00,0x19,0x00,  0x3F,0x00,0x80,     0x40,0x00,0x12,0x01};
//uint8_t const ADXL_CONFIG_DETECT[] = {ADXL_CMD_WRITE_REG,ADXL_REG_THRESH_ACT_L,0x10,0x00,0x05, 0x90,0x00,0x19,0x00,  0x3F,0x00,0x80,     0x40,0x00,0x12,0x06};
//uint8_t const ADXL_CONFIG_DETECT[] = {ADXL_CMD_WRITE_REG,ADXL_REG_THRESH_ACT_L,0xA,0x00,0x02, 0x40,0x00,0x1E,0x00,  0x3F,0x00,0x80,     0x40,0x00,0x13,0x0A};
uint8_t const ADXL_RESET[] = {ADXL_CMD_WRITE_REG,ADXL_REG_SOFT_RESET,0x52};
uint8_t const ADXL_RESET_F[] = {ADXL_CMD_WRITE_REG,ADXL_REG_SOFT_RESET,0x00};
uint8_t const ADXL_CONFIG_READ[] = {ADXL_CMD_READ_REG,ADXL_REG_THRESH_ACT_L,   0x00,0x00,0x00,	0x00,0x00,0x00,0x00,  0x00,0x00,0x00,	 0x00,0x00,0x00,0x00};
uint8_t const ADXL_CONFIG_STBY[] = {ADXL_CMD_WRITE_REG,ADXL_REG_POWER_CTL,0x00}; // Put the ADXL into standby mode

//=============================================================================
//									Functions
//=============================================================================
/**
 * Turn on and start up the ADXL362 accelerometer. This leaves the ADXL running.
 */
void initACCEL() {

	/*sleep 5msto allow Accel to startup*/
	lowPowerSleep(LPM_5ms);
	SPI_transaction(gpRxBuf, (uint8_t*)&ADXL_RESET, sizeof(ADXL_RESET),ACCEL_SPI);
	lowPowerSleep(LPM_1ms);
	SPI_transaction(gpRxBuf, (uint8_t*)&ADXL_CONFIG_DETECT, sizeof(ADXL_CONFIG_DETECT),ACCEL_SPI);

	//lowPowerSleep(LPM_1ms);
	//SPI_transaction(gpRxBuf, (uint8_t*)&ADXL_CONFIG_READ, sizeof(ADXL_CONFIG_READ),ACCEL_SPI);
	//lowPowerSleep(LPM_1ms);

}

/**
 * Put the ADXL362 into a lower power standby state without gating power
 * @Note Accel need to stay
 * @todo Implement this function
 */
void ACCEL_standby() {

		lowPowerSleep(LPM_5ms);
		SPI_transaction(gpRxBuf, (uint8_t*)&ADXL_RESET, sizeof(ADXL_RESET),ACCEL_SPI);
		lowPowerSleep(LPM_1ms);
		SPI_transaction(gpRxBuf, (uint8_t*)&ADXL_CONFIG_STBY, sizeof(ADXL_CONFIG_STBY),ACCEL_SPI);
}


/**
 * Grab configure from the ADXL362 accelerometer
 */
BOOL ACCEL_readConfig() {
	//ACCEL_Enable();
	//SPI_transaction(gpRxBuf, (uint8_t*)&ADXL_READ_XYZ_STATUS_8BIT, sizeof(ADXL_READ_XYZ_STATUS_8BIT));
    SPI_transaction(gpRxBuf, (uint8_t*)&ADXL_CONFIG_READ, sizeof(ADXL_CONFIG_READ),ACCEL_SPI);
	//SPI_releasePort();
	return SUCCESS;
}


/**
 * Grab one ADXL_READ_STATUS from the ADXL362 accelerometer
 */
BOOL ACCEL_Status(uint8_t* result) {
    	SPI_transaction(gpRxBuf, (uint8_t*)&ADXL_READ_STATUS, sizeof(ADXL_READ_STATUS),ACCEL_SPI);
		*(result)   = gpRxBuf[2];
	return SUCCESS;
}



/**
 * Grab one sample from the ADXL362 accelerometer
 */
BOOL ACCEL_singleSample(uint8_t* result) {
	//ACCEL_Enable();
	//while(SPI_acquirePort());
	SPI_transaction(gpRxBuf, (uint8_t*)&ADXL_READ_XYZ_STATUS_8BIT, sizeof(ADXL_READ_XYZ_STATUS_8BIT),ACCEL_SPI);

	*(result)   = gpRxBuf[2];
	*(result+1) = gpRxBuf[3];
	*(result+2) = gpRxBuf[4];
	*(result+3) = gpRxBuf[5];
	//SPI_releasePort();
	return SUCCESS;
}
