/*****************************************************************************
 * tempSense.c
 * @breif  temperature sensing funtion
 * @date 10/15/2014
 * @author Yi Zhao, Sensor Systems Lab,UW and Disney Research Pittsburgh
 * @note Vtemp=0.00252*Temp(C)+0.688, so -50~100(oC) result in 0.06~1v
 * 		 ADC sample period is 75us(30 reference, 45 sampling)
 *****************************************************************************/

//=============================================================================
//								Includes
//=============================================================================
#include "../common/globals.h"
#include "tempSense.h"
#include "../common/timer.h"
#include "../common/e-paper.h"

//=============================================================================
//								Variables
//=============================================================================
static volatile uint16_t temp;

//=============================================================================
//								Functions
//=============================================================================
/**
 * @fun 	Sense temerator sensor in MSP430 (single ADC sampling)
 * @note 	Must fine tuning the offset of temerature sensor reading
 *
 */
resultData tempSense(void){
//	uint16_t IntDegF;
	uint16_t volatile IntDegC;
	//uint16_t temp;
	resultData volatile data;

	REFCTL0  |=	REFON | REFVSEL_0;				//1.5v

	ADC10CTL0 |= ADC10SHT_3 | ADC10ON ;//64 sampling cycles=37us
	ADC10CTL1 |= ADC10DIV_7 |ADC10SSEL_3 |ADC10CONSEQ_0 |ADC10SHP;//SMCLK@13.56Mhz/8=1.69Hz
	//ADC10CTL2 |= ADC10SR;
	ADC10MCTL0 |= ADC10SREF_1 | ADC10INCH_10;	//select temperature sensor
	ADC10IE = ADC10IE0;
	delay_us(T_30us);//delay at lease 25us for reference to be stable
	ADC10CTL0 |= ADC10ENC;
	ADC10CTL0 |= ADC10SC;			//start sampling
	__bis_SR_register(LPM4_bits+GIE);
	ADC10CTL0 = 0;//stop ADC
	REFCTL0 &= ~REFON;

	// Temperature in Celsius
	//actual IntDegC = (temp*0.5819-273.0159)*10;
	//format is t*10+50oC*10+offest (the result will have 50oC offet)
	//IntDegC = ((temp*93)>>4)-2308-385;
	//(tempAverage - 630) * 761) / 1024 );
	data.ADC_data = temp;

	//data.y=176-(IntDegC>>1);				//TEST:suppose detected range is half of the Y_range

	//Version: paper (large temp range)
	//IntDegC = ((temp*93)>>4)-2730;
	//data.y=Y_offset-(IntDegC>>2);//

	#warning "temerature offset in different board is different \
			  must adjust it for your board here"
	IntDegC = ((temp*93)>>4)-2850; 			//2860 is the offset for some board , 2650 for other board
	data.y=Y_offset-(IntDegC>>1);//

	//data.x = senseState + X_start;
	data.x = senseState + 3;

	//TODO what should we do if the Y is out of the range
	if(!((Y_start<data.y)&(data.y<Y_end))){
		data.y = Y_start;				//if out of range than plot on the outline
	}
	if(data.x>=X_end){
		data.x= X_end;
		//TODO clear E-ink Display and redraw
		imageUpdateState=IMG_UPDATE;
		//temp = 0;
		senseState =  X_end-1;
	}
	// Temperature in Fahrenheit
	//actual IntDegF = (temp*1.0474 - 241.0159)*10
	//format is t*10+1220
	//IntDegF = (temp*670)>>6 - 1910;

	return data;
}

//=============================================================================
//								ISR
//=============================================================================
#pragma vector=ADC10_VECTOR
__interrupt void ADC10ISR (void)
{
	if(ADC10IFG&ADC10IFG0){
		temp = ADC10MEM0;
		__bic_SR_register_on_exit(LPM4_bits);   // Exit active CPU
	}
}
