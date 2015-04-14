/*
 * temSense.h
 *
 *  Created on: Nov 17, 2014
 *      Author: evezhao
 */

#ifndef TEMPSENSE_H_
#define TEMPSENSE_H_

/* Screen range for 10~40 temperature range is:
 * (start from 0)
 * 		X= 28  (col3~col31,total is 0~32)
 * 		Y= 152 (line5~line156)
 */

/*Version: Paper (large range)
//test supporse temp range is half of the Y_range
#define Y_start   7
#define Y_end	  156
#define Y_range	 (Y_end - Y_start)   //150 points *4= 600 can represent -10~50oC,
#define Y_offset  123  				 //the y axis start from -10oC 156-102=54/27~26
#define X_start   3
#define X_end	  31
#define X_range	 (X_end - X_start)*8 //right now is 30 samples dot30*8=224 cycles suppose min
*/


//Version: Demo (small temp range)
//test supporse temp range is half of the Y_range
#define Y_start   5
#define Y_end	  156
#define Y_range	 (Y_end - Y_start+1)   //152  *4= 608 can represent 10~40oC,
#define Y_offset  206  				   //the y axis start from 10oC =156 + (10oC)=156 + 100/2 = 206
#define X_start   3
#define X_end	  31
#define X_range	 (X_end - X_start)*8 //right now is 30 samples dot30*8=224 cycles suppose min
//*/
typedef struct{
	uint8_t y;  //lines number
	uint8_t x;  //byte position in E-ink
	uint16_t ADC_data;
	//uint8_t value; // the image value
}resultData;

resultData tempSense(void);
void updateSense(void);
#endif /* TEMSENSE_H_ */
