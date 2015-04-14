/*****************************************************************************
 * @file    NFC_WISP_Rev1.0.h
 * @fun     MSP430 I/O defination and initialization
 * @date    10/25/2014
 * @author  Eve (Yi Zhao), Sensor System Lab, UW
 *****************************************************************************/

#ifndef NFC_WISP_REV1_0_DRP_H_
#define NFC_WISP_REV1_0_DRP_H_

#include <msp430f5310.h>
//=============================================================================
//                                  Port 1    
//=============================================================================
// Pin 1.0 - SW_1
// Interrupting input
#define SW_1_DIR                             P1DIR
#define SW_1_IN                              P1IN
#define SW_1_IFG                             P1IFG
#define SW_1_IES                             P1IES
#define SW_1_IE                              P1IE
#define SW_1_BIT                             BIT0

// Pin 1.1 - SW_2
// Interrupting input
#define SW_2_DIR                             P1DIR
#define SW_2_IN                              P1IN
#define SW_2_IFG                             P1IFG
#define SW_2_IES                             P1IES
#define SW_2_IE                              P1IE
#define SW_2_BIT                             BIT1

// Pin 1.2 - RX_WAKEUP
// Interrupting input
#define RX_WAKEUP_DIR                        P1DIR
#define RX_WAKEUP_IN                         P1IN
#define RX_WAKEUP_IFG                        P1IFG
#define RX_WAKEUP_IES                        P1IES
#define RX_WAKEUP_IE                         P1IE
#define RX_WAKEUP_BIT                        BIT2

// Pin 1.3 - RX
// Interrupting input with peripheral routing
#define RX_DIR                               P1DIR
#define RX_SEL								 P1SEL
#define RX_IN                                P1IN
#define RX_IFG                               P1IFG
#define RX_IES                               P1IES
#define RX_IE                                P1IE
#define RX_BIT                               BIT3

// Pin 1.4 - EPD_PWM
// Output with peripheral routing
#define EPD_PWM_DIR                          P1DIR
#define EPD_PWM_OUT                          P1OUT
#define EPD_PWM_SEL                          P1SEL
#define EPD_PWM_BIT                          BIT4

// Pin 1.5 - Batt_Full
// Interrupting input when battery is fully charged
#define BATT_FULL_DIR                        P1DIR
#define BATT_FULL_IN                         P1IN
#define BATT_FULL_IFG                        P1IFG
#define BATT_FULL_IES                        P1IES
#define BATT_FULL_IE                         P1IE
#define BATT_FULL_BIT                        BIT5

// Pin 1.6 - ACCEL_INT2
// Interrupting input from Accelerometer
#define ACCEL_INT2_DIR                       P1DIR
#define ACCEL_INT2_IN                        P1IN
#define ACCEL_INT2_IFG                       P1IFG
#define ACCEL_INT2_IES                       P1IES
#define ACCEL_INT2_IE                        P1IE
#define ACCEL_INT2_BIT                       BIT6
 
// Pin 1.7 - ACCEL_INT1
// Interrupting input from Accelerometer
#define ACCEL_INT1_DIR                       P1DIR
#define ACCEL_INT1_IN                        P1IN
#define ACCEL_INT1_IFG                       P1IFG
#define ACCEL_INT1_IES                       P1IES
#define ACCEL_INT1_IE                        P1IE
#define ACCEL_INT1_BIT                       BIT7

//------------------------------------------------------------
// Pin below can be reused by disable function in hardware              
//------------------------------------------------------------
// Pin 1.0 - SW_1
// Pin 1.1 - SW_1
// Pin 1.4 - EPD_PWM
// Pin 1.5 - Batt_Full
// Pin 1.6 - ACCEL_INT2
// Pin 1.7 - ACCEL_INT1
//=============================================================================
//                                  Port 2    
//=============================================================================
// Pin 2.0 - VRECT_LOW
// Interrupting input to indicate Vrect below a certain threshold
#define VRECT_LOW_DIR                        P2DIR
#define VRECT_LOW_IN                         P2IN
#define VRECT_LOW_IFG                        P2IFG
#define VRECT_LOW_IES                        P2IES
#define VRECT_LOW_IE                         P2IE
#define VRECT_LOW_BIT                        BIT0

//=============================================================================
//                                  Port 4    
//=============================================================================
#define PSEL_SPI                             P4SEL
#define PDIR_SPI                             P4DIR
#define POUT_SPI                             P4OUT

// Pin 4.0 - SPI_CS_MEM
// Output
#define SPI_CS_MEM_DIR                       P4DIR
#define SPI_CS_MEM_OUT                       P4OUT
#define SPI_CS_MEM_BIT                       BIT0

// Pin 4.1 - SPI_MOSI
// Output with peripheral routing
#define SPI_MOSI_DIR                         P4DIR
#define SPI_MOSI_OUT                         P4OUT
#define SPI_MOSI_SEL                         P4SEL
#define SPI_MOSI_BIT                         BIT1

// Pin 4.2 - SPI_MISO
// Input with peripheral routing
#define SPI_MISO_DIR                         P4DIR
#define SPI_MISO_IN                          P4IN
#define SPI_MISO_SEL                         P4SEL
#define SPI_MISO_BIT                         BIT2

// Pin 4.3 - SPI_CLK
// Output with peripheral routing
#define SPI_CLK_DIR                          P4DIR
#define SPI_CLK_OUT                          P4OUT
#define SPI_CLK_SEL                          P4SEL
#define SPI_CLK_BIT                          BIT3

// Pin 4.4 - LED_1
// Output
#define LED_1_DIR                            P4DIR
#define LED_1_OUT                            P4OUT
#define LED_1_BIT                            BIT4

// Pin 4.5 - RX_UART (RX_P4.5)
// Can be re-used for UART out of doNFC routine
// Internally connect to RX_P1.3 in doNFC routine
#define RX_UART_DIR                          P4DIR
#define RX_UART_IN                           P4IN
#define RX_UART_SEL                          P4SEL
#define RX_UART_BIT                          BIT5

// Pin 4.6 - SPI_CS_EPD
// Output
#define SPI_CS_EPD_DIR                       P4DIR
#define SPI_CS_EPD_OUT                       P4OUT
#define SPI_CS_EPD_BIT                       BIT6

// Pin 4.7 - TX_PWM
// Output with peripheral routing
#define TX_PWM_DIR                           P4DIR
#define TX_PWM_OUT                           P4OUT
#define TX_PWM_SEL                           P4SEL
#define TX_PWM_BIT                           BIT7

//------------------------------------------------------------
// Pin below can be reused by disable block in hardware              
//------------------------------------------------------------
// Pin 4.0 - SPI_CS_MEM
// Pin 4.4 - LED_1
// Pin 4.5 - RX_P4.5 @Note: can not be reused inside RX routine

//=============================================================================
//                                  Port 5    
//=============================================================================
// Pin 5.0 - BATT_MEAS_ENABLE
// Output
#define BATT_MEAS_EN_DIR                     P5DIR
#define BATT_MEAS_EN_OUT                     P5OUT
#define BATT_MEAS_EN_BIT                     BIT0

// Pin 5.1 - LED_2
// Output
#define LED_2_DIR                            P5DIR
#define LED_2_OUT                            P5OUT
#define LED_2_BIT                            BIT1

// Pin 5.2 - XT2IN
// Input with peripheral routing
#define XT2IN_DIR                            P5DIR
#define XT2IN_IN                             P5IN
#define XT2IN_SEL                            P5SEL
#define XT2IN_BIT                            BIT2

// Pin 5.3 - XT2OUT
// Output with peripheral routing
#define XT2OUT_DIR                           P5DIR
#define XT2OUT_OUT                           P5OUT
#define XT2OUT_SEL                           P5SEL
#define XT2OUT_BIT                           BIT3

// Pin 5.4 - XT1IN
// Input with peripheral routing
#define XT1IN_DIR                            P5DIR
#define XT1IN_IN                             P5IN
#define XT1IN_SEL                            P5SEL
#define XT1IN_BIT                            BIT4

// Pin 5.5 - XT1OUT
// Output with peripheral routing
#define XT1OUT_DIR                           P5DIR
#define XT1OUT_OUT                           P5OUT
#define XT1OUT_SEL                           P5SEL
#define XT1OUT_BIT                           BIT5

//------------------------------------------------------------
// Pin below can be reused by disable block in hardware              
//------------------------------------------------------------
// Pin 5.0 - BATT_MEAS_ENABLE
// Pin 5.1 - LED_2

//=============================================================================
//                                  Port 6    
//=============================================================================
// Pin 6.0 - RX_WAKEUP_EN
// Output
#define RX_WAKEUP_EN_DIR                     P6DIR
#define RX_WAKEUP_EN_OUT                     P6OUT
#define RX_WAKEUP_EN_BIT                     BIT0

// Pin 6.1 - RX_ENABLE
// Output
#define RX_ENABLE_DIR                        P6DIR
#define RX_ENABLE_OUT                        P6OUT
#define RX_ENABLE_BIT                        BIT1

// Pin 6.2 - BATT_MEAS
// Input
#define BATT_MEAS_DIR                        P6DIR
#define BATT_MEAS_IN                         P6IN
#define BATT_MEAS_BIT                        BIT2

// Pin 6.3 - SPI_CS_ACCEL
// Output 
#define SPI_CS_ACCEL_DIR                        P6DIR
#define SPI_CS_ACCEL_OUT                        P6OUT
#define SPI_CS_ACCEL_BIT                        BIT3

//------------------------------------------------------------
// Pin below can be reused by disable block in hardware              
//------------------------------------------------------------
// Pin 6.2 - BATT_MEAS
// Pin 6.3 - SPI_CS_ACCEL

//=============================================================================
//                                  Port J    
//=============================================================================
// Pin J.0 - EPD_PANEL_ON
// Output
#define EPD_PANEL_ON_DIR                     PJDIR
#define EPD_PANEL_ON_OUT                     PJOUT
#define EPD_PANEL_ON_BIT                     BIT0

// Pin J.1 - EPD_RESET
// Output
#define EPD_RESET_DIR                        PJDIR
#define EPD_RESET_OUT                        PJOUT
#define EPD_RESET_BIT                        BIT1

// Pin J.2 - EPD_BUSY
// Input
#define EPD_BUSY_DIR                         PJDIR
#define EPD_BUSY_IN                          PJIN
#define EPD_BUSY_BIT                         BIT2

// Pin J.3 - EPD_DISCH
// Output
#define EPD_DISCH_DIR                        PJDIR
#define EPD_DISCH_OUT                        PJOUT
#define EPD_DISCH_BIT                        BIT3
//=============================================================================
//                   I/O initialization when MSP430 start
//=============================================================================
//Note@eve:compartor ENABLE pin should be high Z, EPD_PANEL_ON BAT_MEASURE_ENABLE should be 0
//SPI_CS pin should be high and other SPI pin should be input
//*P4SEL = SPI_CLK_BIT | SPI_MOSI_BIT | SPI_MISO_BIT | RX_UART_BIT; \*/ comment out this line for test@eve
//turn on wakeup enable bit to make sure enough delay to turn on comparator (need at least 20us)
//Note: 1 disable EPD in initial
//      2 put all comparator enable pin to be input
//  
//Port 1: all input,except EPD_PWM                          Output 0
//Port 2: all input
//Port 4: TX, SPI_CS_EPD,LED1, SPI_CLK,SPI_MOSI             Output 0
//        SPI_CS_MEM                                        Output 1
//        RX_P4.5,SPI_MISO                                  Input
//Port 5: XT1IN,XT1OUT,XT2IN,XT2OUT                         Use PSEL
//        BATT_MEAS_ENABLE,LED_2                            Output 0 
//Port 6: all input,except SPI_CS_ACELL                     Output 1
//Port J: EPD_Panel_on,EPD_RESET,EPD_Discharge              Output 0
//        EPD_Busy                                          Input                
// P6DS = RX_ENABLE_BIT|RX_WAKEUP_EN_BIT; \

 #define setupDflt_IO() \
	P4SEL = RX_UART_BIT; \
    P5SEL = XT1IN_BIT | XT1OUT_BIT | XT2IN_BIT | XT2OUT_BIT; \
    P1OUT = 0;\
    P2OUT = 0; \
    P4OUT = SPI_CS_MEM_BIT;\
    P5OUT = 0; \
    P6OUT = RX_WAKEUP_EN_BIT|RX_ENABLE_BIT|SPI_CS_ACCEL_BIT; \
    PJOUT = 0;\
    P1DIR = 0x00; \
    P2DIR = 0; \
    P4DIR = 0xFF&(~(SPI_MISO_BIT|RX_UART_BIT)); \
    P5DIR = BATT_MEAS_EN_BIT|LED_2_BIT; \
    P6DIR = SPI_CS_ACCEL_BIT;\
    PJDIR = EPD_PANEL_ON_BIT|EPD_RESET_BIT|EPD_DISCH_BIT; \
    P1IES |= RX_BIT;//falling edge for RX_BIT and others rising edge

#endif/**/
