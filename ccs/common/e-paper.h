/******************************************************************************
 * e-paper.c
 *
 * @author Artem Dementyev, Pervasive Displays staff
 *
 *****************************************************************************/

#include "globals.h"

typedef enum {
	EPD_1_44,        // 128 x 96
	EPD_2_0,         // 200 x 96
	EPD_2_7          // 264 x 176
} EPD_size;

typedef enum {           // Image pixel -> Display pixel
	EPD_compensate,  // B -> W, W -> B (Current Image)
	EPD_white,       // B -> N, W -> W (Current Image)
	EPD_inverse,     // B -> N, W -> B (New Image)
	EPD_normal       // B -> B, W -> W (New Image)
} EPD_stage;

typedef struct {
	EPD_size size ;
	unsigned int lines_per_display;
	unsigned int dots_per_line;
	unsigned int bytes_per_line;
	unsigned int bytes_per_scan;
	const unsigned char *gate_source;
	unsigned int gate_source_length;

	bool filler;
} EPD_type;



#define CLK_MHz 16
#define SPI_BPS 10000000


#define PORT_INIT   0

//TODO:Derek Thrasher - Not sure how to port this
//version 0.3 this is RX_WAKEUP P1.2
//#define TIMER1_OUT1_BIT BIT2
//#define TIMER1_IO_ENABLE (TIMER1_OUT1_BIT)

#define TIMER1_COUNTS 4000

#define SPI_BR_VALUE (1000000 * CLK_MHz / SPI_BPS)
#define SPI_BR_HIGH ((SPI_BR_VALUE >> 8) & 0xff)
#define SPI_BR_LOW ((SPI_BR_VALUE >> 0) & 0xff)

#if SPI_BR_VALUE <= 0
#error SPI_BPS is too high
#endif

#define EPD_FRAME_CYCLES 4

//#define SPI_CLK_BIT BIT3
//#define SPI_SOMI BIT2
//#define SPI_SIMO BIT1


//function prototypes
void updateDisplay(unsigned char* imageBuffer, EPD_type* cog);
void EPD_initialize(EPD_type *cog);
void EPD_frame_fixed(EPD_type *cog, unsigned char fixed_value);
void EPD_line(EPD_type *cog, unsigned int line, const unsigned char *data, unsigned char fixed_value, EPD_stage stage);
void EPD_finalise(EPD_type *cog);
void EPD_frame_data(EPD_type *cog, const unsigned char *image, EPD_stage stage);

void Delay_us(unsigned int delay);
void Delay_ms(unsigned int delay);
void SetVcoreUp (unsigned int level);

void PWM_start(void);
void PWM_stop(void);

void SPI_initialize(void);
void SPI_put(unsigned char c);
void SPI_put_wait(unsigned char c);
void SPI_send(const unsigned char *buffer, unsigned int length);

