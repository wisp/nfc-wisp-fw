/*****************************************************************************
 * globals.h
 *
 * @date Mar 11, 2013
 * @author Jeremy Gummeson, Derek Thrasher (UMass Amherst)
 *****************************************************************************/

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <stdint.h>
#include <msp430.h>

//=============================================================================
//									Configuration
//=============================================================================

//---SELECT PROTOCOL------------//
//#define _15693_1of256
#define _14443_B

//---SELECT CLOCK FREQUENCY-----//
//#define _6_78MHz
#define _13_56MHz

//=============================================================================
//									Pin definitions
//=============================================================================

// Pin 1.0 - SW_1
// Interrupting input
#define SW_1_DIR                             P1DIR
#define SW_1_IN                              P1IN
#define SW_1_IFG                             P1IFG
#define SW_1_IES                             P1IES
#define SW_1_IE                              P1IE
#define SW_1_BIT                             BIT0

// Pin 1.1 - LED_1
// Output
#define LED_1_DIR                            P1DIR
#define LED_1_OUT                            P1OUT
#define LED_1_BIT                            BIT1

// Pin 1.2 - RX_WAKEUP
// Interrupting input
#define RX_WAKEUP_DIR                        P1DIR
#define RX_WAKEUP_IN                         P1IN
#define RX_WAKEUP_IFG                        P1IFG
#define RX_WAKEUP_IES                        P1IES
#define RX_WAKEUP_IE                         P1IE
#define RX_WAKEUP_BIT                        BIT2

// Pin 1.3 - RX
// Interrupting input
#define RX_DIR                               P1DIR
#define RX_IN                                P1IN
#define RX_IFG                               P1IFG
#define RX_IES                               P1IES
#define RX_IE                                P1IE
#define RX_BIT                               BIT3

// Pin 1.3 - RX_CAP
// Interrupting input with peripheral routing
#define RX_CAP_DIR                           P1DIR
#define RX_CAP_IN                            P1IN
#define RX_CAP_SEL                           P1SEL
#define RX_CAP_IFG                           P1IFG
#define RX_CAP_IES                           P1IES
#define RX_CAP_IE                            P1IE
#define RX_CAP_BIT                           BIT3

// Pin 1.5 - SW_2
// Interrupting input
#define SW_2_DIR                             P1DIR
#define SW_2_IN                              P1IN
#define SW_2_IFG                             P1IFG
#define SW_2_IES                             P1IES
#define SW_2_IE                              P1IE
#define SW_2_BIT                             BIT5

// Pin 1.7 - LED_2
// Output
#define LED_2_DIR                            P1DIR
#define LED_2_OUT                            P1OUT
#define LED_2_BIT                            BIT7

// Pin 2.0 - EPD_PWM
// Output with peripheral routing
#define EPD_PWM_DIR                          P2DIR
#define EPD_PWM_OUT                          P2OUT
#define EPD_PWM_SEL                          P2SEL
#define EPD_PWM_BIT                          BIT0

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

// Pin 4.4 - EPD_BUSY
// Input
#define EPD_BUSY_DIR                         P4DIR
#define EPD_BUSY_IN                          P4IN
#define EPD_BUSY_BIT                         BIT4

// Pin 4.5 - RX_UART
// Input with peripheral routing
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

// Pin 5.0 - RX_ENABLE
// Output
#define RX_ENABLE_DIR                        P5DIR
#define RX_ENABLE_OUT                        P5OUT
#define RX_ENABLE_BIT                        BIT0

// Pin 5.1 - DEBUG
// Output
#define DEBUG_DIR                            P5DIR
#define DEBUG_OUT                            P5OUT
#define DEBUG_BIT                            BIT1

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

// Pin 6.0 - LCE
// Output
#define LCE_DIR                              P6DIR
#define LCE_OUT                              P6OUT
#define LCE_BIT                              BIT0

// Pin 6.1 - BATT_MEAS
// Input
#define BATT_MEAS_DIR                        P6DIR
#define BATT_MEAS_IN                         P6IN
#define BATT_MEAS_BIT                        BIT1

// Pin 6.2 - BATT_MEAS_EN
// Output
#define BATT_MEAS_EN_DIR                     P6DIR
#define BATT_MEAS_EN_OUT                     P6OUT
#define BATT_MEAS_EN_BIT                     BIT2

// Pin 6.3 - RX_WAKEUP_EN
// Output
#define RX_WAKEUP_EN_DIR                     P6DIR
#define RX_WAKEUP_EN_OUT                     P6OUT
#define RX_WAKEUP_EN_BIT                     BIT3

// Pin J.0 - EPD_DISCH
// Output
#define EPD_DISCH_DIR                        PJDIR
#define EPD_DISCH_OUT                        PJOUT
#define EPD_DISCH_BIT                        BIT0

// Pin J.1 - EPD_PANEL_ON
// Output
#define EPD_PANEL_ON_DIR                     PJDIR
#define EPD_PANEL_ON_OUT                     PJOUT
#define EPD_PANEL_ON_BIT                     BIT1

// Pin J.2 - EPD_RESET
// Output
#define EPD_RESET_DIR                        PJDIR
#define EPD_RESET_OUT                        PJOUT
#define EPD_RESET_BIT                        BIT2


//=============================================================================
//									General Defines
//=============================================================================

#define CMD_BUF_SIZE 64

#define CAPTURE_BUFFER_SIZE	10  //maximum number of pulse capture times we can store

#define CRYSTAL_FREQ	32768
#define TIMEOUT_MS		100   //low power timeout in ms
#define TIMEOUT_VAL		(CRYSTAL_FREQ/(TIMEOUT/10))

#define E_INK_SIZE 5808 //264*176/8

#define LED_1_PULSE_FLAG	0x01
#define LED_2_BIT_PULSE_FLAG	0x02
#define KILL_LEDS_FLAG		0x80


//=============================================================================
//									Functions
//=============================================================================
void system_initialize(void);
void timeout_initialize(uint16_t); //used for power management
void led_1_auto_pulse();
void LED_2_BIT_auto_pulse();


//=============================================================================
//									Macros
//=============================================================================

// LED Macros
#define toggle_led_1() P1OUT ^= LED_1_BIT
#define toggle_led_2() P1OUT ^= LED_2_BIT
#define led_1_off() P1OUT &= ~(LED_1_BIT)
#define led_2_off() P1OUT &= ~(LED_2_BIT)
#define led_1_on() P1OUT |= LED_1_BIT
#define led_2_on() P1OUT |= LED_2_BIT



//=============================================================================
//									Globals
//=============================================================================
extern uint8_t imageBuffer[E_INK_SIZE];
extern uint8_t imageTransferDoneFlag;
extern uint16_t bad_interrupts;
extern uint8_t led_pulse_flags;

#endif
