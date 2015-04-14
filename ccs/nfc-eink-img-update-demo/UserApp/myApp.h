/******************************************************************************
 ** myApp.h
 * @fun	 Demo for Temperature/Acceleration display&logging
 * @author Yi Zhao(Eve), Sensor System Lab,UW and Disney Research Pittsburgh
 * @date -09/29/2014
 *****************************************************************************/
#ifndef MYAPP_H_
#define MYAPP_H_

//=============================================================================
//						Define
//=============================================================================
//Buffer if needed for data logger
#define BUF_SIZE		2//2600//4000

//=============================================================================
//						Micros
//=============================================================================
// We must disable sensor ISR in NFC rx&tx routine
#warning "Must disable sensor interrupt here for different Usesr App\
		  otherwise there is a conflict with NFC reading ISR"
//implement "enable or disable custermized ISR" for UserApp
#define disable_sensor_ISR() \
		TA1CCTL0 = 0;
		//ACCEL_INT1_IE &= ~ACCEL_INT1_BIT;

#define enable_sensor_ISR() \
		TA1CCTL0 |= CCIE;
		//ACCEL_INT1_IE |= ACCEL_INT1_BIT;


//=============================================================================
//						myApp Globals
//=============================================================================
extern uint8_t 	volatile 		doNFC_state;
extern volatile unsigned long 	RTC_ctr;
extern uint8_t volatile			imageUpdateState;
extern volatile uint8_t			senseState;
extern volatile uint8_t			accelState;
extern uint8_t 					sense_buf[BUF_SIZE];
extern uint8_t* 				sense_buf_ptr;
extern uint8_t* 				sense_read_ptr;
extern uint8_t	 				buf_full;
extern uint16_t 				senseCtr;
//extern const char[5808]	templete;

//=============================================================================
//                      Function
//=============================================================================
void initMyDemo1(void);
void initMyDemo2(void);
void Demo1(void);
void Demo2(void);
void Demo3(void);


#endif /* MYAPP_H_ */
