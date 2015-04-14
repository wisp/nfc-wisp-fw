/**
 * accel.h
 *
 *  @date Aug 2013
 *  @author Aaron Parks, Eve(Yi Zhao), Sensor Systems Lab, UW
 */

#ifndef ACCEL_H_
#define ACCEL_H_

#include "globals.h"

typedef struct {
	uint16_t x;
	uint16_t y;
	uint16_t z;
} threeAxis_t;

void initACCEL(void);
void ACCEL_standby(void);
BOOL ACCEL_Status(uint8_t* result);
//BOOL ACCEL_satus(threeAxis_t* result);
BOOL ACCEL_singleSample(uint8_t* result);
BOOL ACCEL_Status(uint8_t* result);
BOOL activity(uint8_t* preACC);

//ACCEL I/O Macros
#define ACCEL_Enable()	\
	//;
	//POUT_EN_ACCEL |=  PIN_ACCEL_CS; \
	//PDIR_EN_ACCEL |=  PIN_ACCEL_CS;
#define ACCEL_Disable()	\
	;
	//PDIR_EN_ACCEL &= ~PIN_EN_ACCEL + PIN_ACCEL_CS;
#endif /* ACCEL_H_ */
