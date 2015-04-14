/*****************************************************************************
 * globals.c
 * @breif  global functions
 * @date 8/15/2014
 * @author Yi Zhao, Uw Sensor systems Lab
 *****************************************************************************/
//-----------------------------------------------------------------------------
//							Includes
//------------------------------------------------------------------------------
#include "globals.h"

//------------------------------------------------------------------------------
//							Functions
//------------------------------------------------------------------------------
/**
 * Configure LED 1 to pulse once without further intervention
 */
void led_1_auto_pulse(void) {
	led_pulse_flags|=LED_1_PULSE_FLAG;
}

/**
 * Configure LED 2 to pulse once without further intervention
 */
void LED_2_BIT_auto_pulse(void) {
	led_pulse_flags|=LED_2_BIT_PULSE_FLAG;
}


