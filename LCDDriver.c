/*****************************************************************
*  isim : Nokia 6100 lcd (PCF8833) sürücü                        *
*  Düzenleyen/Geliþtiren : Erhan YILMAZ		                     *
*  tarih   : 26.09.2010                                          *
*  Sürüm   : 1.0   
*  Hafýza kullanýmý : 2768 byte flash ,101 byte ram              *
*****************************************************************/

// 	http://www.nxp.com/acrobat_download/datasheets/PCF8833_1.pdf

// ------------------------------------------------------------------------------
//
//
// The LCD controller can be configured for different coordinate systems.  I choose
// to mount the display with the connector tab on the top, and I setup the coordinate
// system with 0,0 in the upper left corner.  X moves down and Y moves to the right.
// With this display, the X=0 and X=131 columns, along with the Y=0 and Y = 131 rows 
// are not visible and are not used.  
//
//
//                     ----------- Y ----------->
//                                        ___
//                    _ _ _ _ _ _ _ _ _ _|_ _|_ _
//                   |                           |
//             (0,0) |---------------------------| (131,0)
//        |          |                           |
//        |          |                           |
//        |          |                           |
//        |          |                           |
//        |          |    Nokia 6100 display     |
//        X          |                           |
//        |          |                           |
//        |          |                           |
//        |          |                           |
//        |          |                           |
//       \|/         |                           |
//           (0,131)  ---------------------------  (131,131)
//
//
// The auto increment and wrap feature of the RAMWR command is configured
// to write bytes in the Y direction, going from top to bottom, then wrapping  
// left to right. As a result, bitmap data should be stored in arrays with the  
// first byte representing the pixel in the upper left corner, then advancing 
// downward and to the right.
//

/* ------------------------------------------------------------------------ */


#include <sys/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "LCDDriver.h"

#define BASEPORT 0x378
/* ------------------------------------------------------------------------ */


//
// command set for the Philips PCF8833 LCD controller
//
#define NOOP 	0x00 		// nop
#define SWRESET 0x01 		// software reset
#define BSTROFF 0x02 		// booster voltage OFF
#define BSTRON 	0x03 		// booster voltage ON
#define RDDIDIF 0x04 		// read display identification
#define RDDST 	0x09 		// read display status
#define SLEEPIN 0x10 		// sleep in
#define SLEEPOUT 0x11 		// sleep out
#define PTLON 	0x12 		// partial display mode
#define NORON 	0x13 		// display normal mode
#define INVOFF 	0x20 		// inversion OFF
#define INVON 	0x21 		// inversion ON
#define DALO 	0x22 		// all pixel OFF
#define DAL 	0x23 		// all pixel ON
#define SETCON 	0x25 		// write contrast
#define DISPOFF 0x28 		// display OFF
#define DISPON 	0x29 		// display ON
#define CASET 	0x2A 		// column address set
#define PASET 	0x2B 		// page address set
#define RAMWR 	0x2C 		// memory write
#define RGBSET 	0x2D 		// colour set
#define PTLAR 	0x30 		// partial area
#define VSCRDEF 0x33 		// vertical scrolling definition
#define TEOFF 	0x34 		// test mode
#define TEON 	0x35		// test mode
#define MADCTL 	0x36 		// memory access control
#define SEP 	0x37 		// vertical scrolling start address
#define IDMOFF 	0x38 		// idle mode OFF
#define IDMON 	0x39 		// idle mode ON
#define COLMOD 	0x3A 		// interface pixel format
#define SETVOP 	0xB0 		// set Vop
#define BRS 	0xB4 		// bottom row swap
#define TRS 	0xB6 		// top row swap
#define DISCTR 	0xB9 		// display control
#define DATOR 	0xBA 		// data order
#define TCDFE 	0xBD 		// enable/disable DF temperature compensation
#define TCVOPE 	0xBF 		// enable/disable Vop temp comp
#define EC 		0xC0 		// internal or external oscillator
#define SETMUL 	0xC2 		// set multiplication factor
#define TCVOPAB 0xC3 		// set TCVOP slopes A and B
#define TCVOPCD 0xC4 		// set TCVOP slopes c and d
#define TCDF 	0xC5 		// set divider frequency
#define DF8COLOR 0xC6 		// set divider frequency 8-color mode
#define SETBS 	0xC7 		// set bias system
#define RDTEMP 	0xC8 		// temperature read back
#define NLI 	0xC9 		// n-line inversion
#define RDID1 	0xDA 		// read ID1
#define RDID2 	0xDB 		// read ID2
#define RDID3 	0xDC 		// read ID3


const uint8_t Font5x7[] = {
	0x00, 0x00, 0x00, 0x00, 0x00,// (space)
	0x00, 0x00, 0x5F, 0x00, 0x00,// !
	0x00, 0x07, 0x00, 0x07, 0x00,// "
	0x14, 0x7F, 0x14, 0x7F, 0x14,// #
	0x24, 0x2A, 0x7F, 0x2A, 0x12,// $
	0x23, 0x13, 0x08, 0x64, 0x62,// %
	0x36, 0x49, 0x55, 0x22, 0x50,// &
	0x00, 0x05, 0x03, 0x00, 0x00,// '
	0x00, 0x1C, 0x22, 0x41, 0x00,// (
	0x00, 0x41, 0x22, 0x1C, 0x00,// )
	0x08, 0x2A, 0x1C, 0x2A, 0x08,// *
	0x08, 0x08, 0x3E, 0x08, 0x08,// +
	0x00, 0x50, 0x30, 0x00, 0x00,// ,
	0x08, 0x08, 0x08, 0x08, 0x08,// -
	0x00, 0x60, 0x60, 0x00, 0x00,// .
	0x20, 0x10, 0x08, 0x04, 0x02,// /
	0x3E, 0x51, 0x49, 0x45, 0x3E,// 0
	0x00, 0x42, 0x7F, 0x40, 0x00,// 1
	0x42, 0x61, 0x51, 0x49, 0x46,// 2
	0x21, 0x41, 0x45, 0x4B, 0x31,// 3
	0x18, 0x14, 0x12, 0x7F, 0x10,// 4
	0x27, 0x45, 0x45, 0x45, 0x39,// 5
	0x3C, 0x4A, 0x49, 0x49, 0x30,// 6
	0x01, 0x71, 0x09, 0x05, 0x03,// 7
	0x36, 0x49, 0x49, 0x49, 0x36,// 8
	0x06, 0x49, 0x49, 0x29, 0x1E,// 9
	0x00, 0x36, 0x36, 0x00, 0x00,// :
	0x00, 0x56, 0x36, 0x00, 0x00,// ;
	0x00, 0x08, 0x14, 0x22, 0x41,// <
	0x14, 0x14, 0x14, 0x14, 0x14,// =
	0x41, 0x22, 0x14, 0x08, 0x00,// >
	0x02, 0x01, 0x51, 0x09, 0x06,// ?
	0x32, 0x49, 0x79, 0x41, 0x3E,// @
	0x7E, 0x11, 0x11, 0x11, 0x7E,// A
	0x7F, 0x49, 0x49, 0x49, 0x36,// B
	0x3E, 0x41, 0x41, 0x41, 0x22,// C
	0x7F, 0x41, 0x41, 0x22, 0x1C,// D
	0x7F, 0x49, 0x49, 0x49, 0x41,// E
	0x7F, 0x09, 0x09, 0x01, 0x01,// F
	0x3E, 0x41, 0x41, 0x51, 0x32,// G
	0x7F, 0x08, 0x08, 0x08, 0x7F,// H
	0x00, 0x41, 0x7F, 0x41, 0x00,// I
	0x20, 0x40, 0x41, 0x3F, 0x01,// J
	0x7F, 0x08, 0x14, 0x22, 0x41,// K
	0x7F, 0x40, 0x40, 0x40, 0x40,// L
	0x7F, 0x02, 0x04, 0x02, 0x7F,// M
	0x7F, 0x04, 0x08, 0x10, 0x7F,// N
	0x3E, 0x41, 0x41, 0x41, 0x3E,// O
	0x7F, 0x09, 0x09, 0x09, 0x06,// P
	0x3E, 0x41, 0x51, 0x21, 0x5E,// Q
	0x7F, 0x09, 0x19, 0x29, 0x46,// R
	0x46, 0x49, 0x49, 0x49, 0x31,// S
	0x01, 0x01, 0x7F, 0x01, 0x01,// T
	0x3F, 0x40, 0x40, 0x40, 0x3F,// U
	0x1F, 0x20, 0x40, 0x20, 0x1F,// V
	0x7F, 0x20, 0x18, 0x20, 0x7F,// W
	0x63, 0x14, 0x08, 0x14, 0x63,// X
	0x03, 0x04, 0x78, 0x04, 0x03,// Y
	0x61, 0x51, 0x49, 0x45, 0x43,// Z
	0x00, 0x00, 0x7F, 0x41, 0x41,// [
	0x02, 0x04, 0x08, 0x10, 0x20,// "\"
	0x41, 0x41, 0x7F, 0x00, 0x00,// ]
	0x04, 0x02, 0x01, 0x02, 0x04,// ^
	0x40, 0x40, 0x40, 0x40, 0x40,// _
	0x00, 0x01, 0x02, 0x04, 0x00,// `
	0x20, 0x54, 0x54, 0x54, 0x78,// a
	0x7F, 0x48, 0x44, 0x44, 0x38,// b
	0x38, 0x44, 0x44, 0x44, 0x20,// c
	0x38, 0x44, 0x44, 0x48, 0x7F,// d
	0x38, 0x54, 0x54, 0x54, 0x18,// e
	0x08, 0x7E, 0x09, 0x01, 0x02,// f
	0x08, 0x14, 0x54, 0x54, 0x3C,// g
	0x7F, 0x08, 0x04, 0x04, 0x78,// h
	0x00, 0x44, 0x7D, 0x40, 0x00,// i
	0x20, 0x40, 0x44, 0x3D, 0x00,// j
	0x00, 0x7F, 0x10, 0x28, 0x44,// k
	0x00, 0x41, 0x7F, 0x40, 0x00,// l
	0x7C, 0x04, 0x18, 0x04, 0x78,// m
	0x7C, 0x08, 0x04, 0x04, 0x78,// n
	0x38, 0x44, 0x44, 0x44, 0x38,// o
	0x7C, 0x14, 0x14, 0x14, 0x08,// p
	0x08, 0x14, 0x14, 0x18, 0x7C,// q
	0x7C, 0x08, 0x04, 0x04, 0x08,// r
	0x48, 0x54, 0x54, 0x54, 0x20,// s
	0x04, 0x3F, 0x44, 0x40, 0x20,// t
	0x3C, 0x40, 0x40, 0x20, 0x7C,// u
	0x1C, 0x20, 0x40, 0x20, 0x1C,// v
	0x3C, 0x40, 0x30, 0x40, 0x3C,// w
	0x44, 0x28, 0x10, 0x28, 0x44,// x
	0x0C, 0x50, 0x50, 0x50, 0x3C,// y
	0x44, 0x64, 0x54, 0x4C, 0x44,// z
	0x00, 0x08, 0x36, 0x41, 0x00,// {
	0x00, 0x00, 0x7F, 0x00, 0x00,// |
	0x00, 0x41, 0x36, 0x08, 0x00,// }
	0x08, 0x08, 0x2A, 0x1C, 0x08,// ->
	0x08, 0x1C, 0x2A, 0x08, 0x08 // <-
};


/* ------------------------------------------------------------------------ */


//
// color map for RGB12 (rrrrggggbbbb)
//
static  char RGB12ColorMap[] = {
	// number of bytes in the table excluding this one
	48,
	
	// red map: an input 4 bit rrrr color is mapped to an output 5 bit rrrrr color
	0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,0x10,0x12,0x14,0x16,0x18,0x1A,0x1C,0x1F,
	
	// green map: an input 4 bit gggg color is mapped to an output 6 bit gggggg color
	0x00,0x07,0x0B,0x0F,0x13,0x17,0x1B,0x1F,0x23,0x27,0x2B,0x2F,0x33,0x37,0x3B,0x3F,
	
	// blue map: an input 4 bit bbbb color is mapped to an output 5 bit bbbbb color
	0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,0x10,0x12,0x14,0x16,0x18,0x1A,0x1C,0x1F};



//
// color map for RGB8 (rrrgggbb).  This color table does not follow the requirements
// detailed in the Philips datasheet, but does work correctly with the displays used
// to develop this software.  I'm not sure why.
//
static  char RGB8ColorMap[] = {
	// number of bytes in the table excluding this one
	48,

	// red map: an input 3 bit rrr color is mapped to an output 5 bit rrrrr color
	0, 4, 9, 13, 18, 22, 27, 31, 0, 0, 0, 0, 0, 0, 0, 0,
	
	// green map: an input 3 bit ggg color is mapped to an output 6 bit gggggg color
	0, 9, 18, 27, 36, 45, 54, 63, 0, 0, 0, 0, 0, 0, 0, 0,
	
	// blue map: an input 2 bit bb color is mapped to an output 5 bit bbbbb color
	0, 10, 21, 31, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};



//
// color map for RGB8 (rrrgggbb). This color table has not been tested.  It is
// here because it matches the required as documented in the Philips datasheet.
//
//static rom byte RGB8ColorMap[] = {
//	// number of bytes in the table excluding this one
//	20,
//
//	// red map: an input 3 bit rrr color is mapped to an output 4 bit rrrr color
//	0, 2, 5, 7, 9, 11, 14, 15, 
//	
//	// green map: an input 3 bit ggg color is mapped to an output 4 bit gggg color
//	0, 2, 4, 6, 9, 11, 14, 15,
//	
//	// blue map: an input 2 bit bb color is mapped to an output 4 bit bbbb color
//	0, 6, 11, 15};



/* ------------------------------------------------------------------------ */
/*             	    Private variables local to this module					*/
/* ------------------------------------------------------------------------ */

static char CurrentColorMode;
static char *CurrentColorMap;


/* ------------------------------------------------------------------------ */
/*						  Local function declarations  	 					*/
/* ------------------------------------------------------------------------ */

static void SpiInit(void);
static void SpiByteSend(char);
static void GlcdWriteCmd(char);
static void WriteRomDataToLCD( char *, unsigned int);
static void GlcdWriteData(char);
static void DeselectLCD(void);


/* ------------------------------------------------------------------------ */
/*						  	Public general functions 		 				*/
/* ------------------------------------------------------------------------ */

//
// initialize this module and the LCD controller
//		Enter:	ColorMode = LCD_COLOR_MODE_RGB8 or LCD_COLOR_MODE_RGB8
//
void LCDDriverInitialize(char ColorMode)
{	//
	// initialize state variables
	//
	CurrentColorMode = LCD_COLOR_MODE_UNDEFINED;
	CurrentColorMap = 0;
	
	
	//
	// initialize the SPI registers that communicate with the LCD display
	//
	SpiInit();
	//
	// hardware reset the LCD display
	//
        outb(( inb(BASEPORT) &~ GLCD_RESET), BASEPORT );
        usleep( 30000 );
        outb(( inb(BASEPORT) | GLCD_RESET), BASEPORT );
        usleep( 30000 );


	//
	// take the controller out of sleep (start the power booster)
	//
	//SPIData = SLEEPOUT;
	GlcdWriteCmd(SLEEPOUT);
        usleep( 2000 );

	
	//
	// don't mirror x & y, write in Y dir, color = RGB
	// Note: The V bit is set so that writes to RAM increment in the Y direction, going
	// from top to bottom, then left to right.  I picked the Y direction to be compatible
	// with the variable pitch font.  Setting this bit also required switching PASET and 
	// CASET
	//
	//SPIData = MADCTL;
	GlcdWriteCmd(MADCTL);
	//SPIData = 0x20;			
	//GlcdWriteData(0x20);
        GlcdWriteData(0x40); //horizontal ram write, X mirror

	//
	// initialize the contrast
	//
	GlcdWriteCmd(SETCON);
	//GlcdWriteData(0x30);
        GlcdWriteData(0x40);
	DeselectLCD();

	//
	// turn on the display
	//
        usleep( 4000 );
	GlcdWriteCmd(DISPON);		

	//
	// set the default color mode (RGB8 or RGB12) and the color map
	//
	LCDSelectColorMode(ColorMode, 0);
	DeselectLCD();
}

//
// select color mode, either RGB8 or RGB12
//		Enter:	ColorMode = LCD_COLOR_MODE_RGB8 or LCD_COLOR_MODE_RGB8
//				ColorMap -> color map table, if 0 then use default table
//
void LCDSelectColorMode(char ColorMode,  char *ColorMap)
{	 char *ColorMapTable;
	
	//
	// check if the display is already in the current mode
	//
	if ((CurrentColorMode == ColorMode) && (CurrentColorMap == ColorMap)) 
		return;
	// set the color mode
	GlcdWriteCmd(COLMOD);
	
	if (ColorMode == LCD_COLOR_MODE_RGB8)
	{
		GlcdWriteData(2);
		ColorMapTable = RGB8ColorMap;
	}
	else
	{	
		GlcdWriteData(3);
		ColorMapTable = RGB12ColorMap;
	}

	// check if the default table should not be used
	if (ColorMap != 0)
		ColorMapTable = ColorMap;
	// load the color table
	GlcdWriteCmd(RGBSET);
	WriteRomDataToLCD(ColorMapTable + 1, ColorMapTable[0]);
	GlcdWriteCmd(NOOP);
	CurrentColorMode = ColorMode;
	CurrentColorMap = ColorMap;
	DeselectLCD();
}


void GlcdClear(void)
{	if (CurrentColorMode == LCD_COLOR_MODE_RGB8)
		DrawFilledRect_RGB8(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, RGB8_WHITE);
	
	else
		DrawFilledRect_RGB212(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, RGB12_WHITE);
}

//		Contrast (-64 to 63)
//
void SetContrast(char val)
{	
	GlcdWriteCmd(SETCON);
	GlcdWriteData((char) val);
	DeselectLCD();
}

void SetScrolling(void)
{
    GlcdWriteCmd(NORON); //NORON, not partial mode
    //non rolling mode
    //instance row 0 and 131. If a 132 × 132 display is
    //connected to the PCF8833 the content of row 0 and 131
    //will be the same as the content which is displayed in row
    //1 and 130, respectively. By doing so the display data RAM
    //will have 2 rows in the background, whose content can be
    //updated when they are not displayed.

    GlcdWriteCmd(VSCRDEF); //  Vertical scrolling definition (VSCRDEF) command, czy bedzie obrocone gdy bit V=1 w MADCTL?
    GlcdWriteData(0); //zero sta?ych linii od góry
    GlcdWriteData(100); //100 linii scrollowanych
    GlcdWriteData(32); //32 linii nieruchomych na dole
    DeselectLCD();
}
void SetSep(uint8_t sep)
{
    GlcdWriteCmd(SEP); //SEP - scroll entry point
    GlcdWriteData(sep); //sep 0-99, powiekszany o jeden
    DeselectLCD();
}

/* ------------------------------------------------------------------------ */
/*						  	 	 RGB8 functions		 					*/
/* ------------------------------------------------------------------------ */

//

void PutPixel_RGB8(char x, char y, char ColorRGB8)
{	//
	// set the coords of a box including just this one pixel
	//
	GlcdWriteCmd(PASET);
	GlcdWriteData(x);
	GlcdWriteData(x);
	
	GlcdWriteCmd(CASET);
	GlcdWriteData(y);
	GlcdWriteData(y);
	
	// set the color of the pixel
	GlcdWriteCmd(RAMWR);
	GlcdWriteData(ColorRGB8);
	
	DeselectLCD();
}

void DrawFilledRect_RGB8(char Left, char Top, char Right, char Bottom, char ColorRGB8)
{	int i,PixelCount;
	if ((Left > Right) || (Top > Bottom))
		return;
	// Enter rectangle coords
	GlcdWriteCmd(PASET);
	GlcdWriteData(Left);
	GlcdWriteData(Right);
	GlcdWriteCmd(CASET);
	GlcdWriteData(Top);
	GlcdWriteData(Bottom);
	// Fill rectangle
	GlcdWriteCmd(RAMWR);
	PixelCount = (unsigned int) (Right - Left + 1) * (unsigned int) (Bottom - Top + 1);
	for (i = 0; i < PixelCount; i++)
	GlcdWriteData(ColorRGB8);
	GlcdWriteCmd(NOOP);
	DeselectLCD();
}

//
// draw a horizontal gradient
//		Enter:	ColorTable -> to a table in RAM of RGB8 colors
//				LeftX = left side of horizontal gradient to draw
//				RightX = right side of horizontal gradient to draw
//				TopY = Y coord of top line of horizontal gradient to draw, will draw down from there
//				LineCount = number of lines to draw
//
void DrawHorzGradientRGB8( char *ColorTable, char LeftX, char RightX, char TopY, char LineCount)
{	//char i;
	
	while(LineCount)
	{	DrawFilledRect_RGB8(
		  LeftX, TopY, 
		  RightX, TopY, 
		  *ColorTable);

		LineCount--;
		TopY++;
		ColorTable++;
	}
}

void DrawLine_RGB8(int X1, int Y1, int X2, int Y2, char ColorRGB8) 
{	int dy;
	int dx;
	int StepX, StepY;
	int Fraction;
	
	dy = Y2 - Y1;
	dx = X2 - X1;

	if (dy < 0)
	{	dy = -dy; 
		StepY = -1;
	}
	else
		StepY = 1;
		
	if (dx < 0) 
	{	dx = -dx; 
		StepX = -1; 
	}
	else
		StepX = 1;

	dy <<= 1; 									// dy is now 2*dy
	dx <<= 1; 									// dx is now 2*dx
	PutPixel_RGB8(X1, Y1, ColorRGB8);

	if (dx > dy) 
	{	Fraction = dy - (dx >> 1); 				// same as 2*dy - dx
		while (X1 != X2) 
		{	if (Fraction >= 0) 
			{	Y1 += StepY;
				Fraction -= dx; 				// same as fraction -= 2*dx
			}

			X1 += StepX;
			Fraction += dy; 					// same as fraction -= 2*dy
			PutPixel_RGB8(X1, Y1, ColorRGB8);
		}
	} 
	
	else 
	{	Fraction = dx - (dy >> 1);
		while (Y1 != Y2) 
		{	if (Fraction >= 0) 
			{	X1 += StepX;
				Fraction -= dy;
			}

			Y1 += StepY;
			Fraction += dx;
			PutPixel_RGB8(X1, Y1, ColorRGB8);
		}
	}
}

void DrawColumn_RGB8(int Y, char ColorRGB8)
{       
        uint8_t i;
	// Enter rectangle coords
	GlcdWriteCmd(PASET);
	GlcdWriteData(Y);
	GlcdWriteData(Y);
	GlcdWriteCmd(CASET);
	GlcdWriteData(0);
	GlcdWriteData(132);
	// Fill rectangle
	GlcdWriteCmd(RAMWR);
	for (i = 0; i < 132; i++)
        {
            GlcdWriteData(ColorRGB8);
        }
	GlcdWriteCmd(NOOP);
	DeselectLCD();
}

void DrawCircle_RGB8(int X1, int Y1, int Radius, char ColorRGB8) 
{	int f;
	int ddF_x;
	int ddF_y;
	int x;
	int y;
	
	f = 1 - Radius;
	ddF_x = 0;
	ddF_y = -2 * Radius;
	x = 0;
	y = Radius;

	PutPixel_RGB8(X1, Y1 + Radius, ColorRGB8);
	PutPixel_RGB8(X1, Y1 - Radius, ColorRGB8);
	PutPixel_RGB8(X1 + Radius, Y1, ColorRGB8);
	PutPixel_RGB8(X1 - Radius, Y1, ColorRGB8);

	while (x < y) 
	{	if (f >= 0) 
		{	y--;
			ddF_y += 2;
			f += ddF_y;
		}

		x++;
		ddF_x += 2;
		f += ddF_x + 1;
		
		PutPixel_RGB8(X1 + x, Y1 + y, ColorRGB8);
		PutPixel_RGB8(X1 - x, Y1 + y, ColorRGB8);
		PutPixel_RGB8(X1 + x, Y1 - y, ColorRGB8);
		PutPixel_RGB8(X1 - x, Y1 - y, ColorRGB8);
		PutPixel_RGB8(X1 + y, Y1 + x, ColorRGB8);
		PutPixel_RGB8(X1 - y, Y1 + x, ColorRGB8);
		PutPixel_RGB8(X1 + y, Y1 - x, ColorRGB8);
		PutPixel_RGB8(X1 - y, Y1 - x, ColorRGB8);
	}
}

void DrawPixmap_RGB8( char *Bitmap, char X, char Y)
{	char Width;
	char Height;
	int ByteCount;
	
	Width = Bitmap[1];
	Height = Bitmap[2];
	ByteCount = Bitmap[3] + ((int)Bitmap[4] << 8);
	
	GlcdWriteCmd(PASET); 
	GlcdWriteData(X);
	GlcdWriteData(X + Width - 1);
	
	GlcdWriteCmd(CASET);
	GlcdWriteData(Y);
	GlcdWriteData(Y + Height - 1);

	GlcdWriteCmd(RAMWR);
	WriteRomDataToLCD(Bitmap + 5, ByteCount);

	GlcdWriteCmd(NOOP);
	DeselectLCD();
}

void DrawStr_8( const char *s,char X,char Y,char char_color8,char backgr_color8)
{
	while(*s)
		{
		DrawChar_8(*s++,X,Y,char_color8,backgr_color8);
		X=X+6;
		}
		} 

void DrawChar_8(char C ,char X,char Y,char char_color8,char backgr_color8){
	char chardata,column,row,mask;
	GlcdWriteCmd(PASET);
	GlcdWriteData(X);
	GlcdWriteData(X + 4);	
	GlcdWriteCmd(CASET);
	GlcdWriteData(Y);
	GlcdWriteData(Y + 7);
	GlcdWriteCmd(RAMWR);
		for(column=0; column<5; column++){
		chardata=Font5x7[(((C - 0x20) * 5) + column)];
		mask=0x01;
		for (row = 0; row < 8; row++){
		if ((chardata & mask) == 0)
			GlcdWriteData(backgr_color8);
		else
			GlcdWriteData(char_color8);
			mask = mask << 1;}
		}
		GlcdWriteCmd(NOOP);
		DeselectLCD();
		}

/* ------------------------------------------------------------------------ */
/*						  	 	 RGB12 functions 		 				*/
/* ------------------------------------------------------------------------ */


void DrawFilledRect_RGB212(char Left, char Top, char Right, char Bottom, int ColorRGB12)
{	int i;
	int PixelCount;
	char Byte1, Byte2, Byte3;
	int LoopCount;
	
	if ((Left > Right) || (Top > Bottom))
		return;
	GlcdWriteCmd(PASET);
	GlcdWriteData(Left);
	GlcdWriteData(Right);
	GlcdWriteCmd(CASET);
	GlcdWriteData(Top);
	GlcdWriteData(Bottom);
	GlcdWriteCmd(RAMWR);
	
	Byte1 = (ColorRGB12 >> 4) & 0xff;
	Byte2 = (((ColorRGB12 & 0x0f) << 4) | ((ColorRGB12 >> 8) & 0x0f));
	Byte3 = (ColorRGB12 & 0xff);
	
	PixelCount = (unsigned int) (Right - Left + 1) * (unsigned int) (Bottom - Top + 1);
	LoopCount = (PixelCount / 2) + 1;
	for (i = 0; i < LoopCount; i++)
	{	
		GlcdWriteData(Byte1);
		GlcdWriteData(Byte2);
		GlcdWriteData(Byte3);
	}
	GlcdWriteCmd(NOOP);
	DeselectLCD();
}

void DrawPixmap_RGB12( char *Bitmap, char X, char Y)
{	char Width;
	char Height;
	int ByteCount;
	
	Width = Bitmap[1];
	Height = Bitmap[2];
	ByteCount = Bitmap[3] + ((int)Bitmap[4] << 8);
	GlcdWriteCmd(PASET);
	GlcdWriteData(X);
	GlcdWriteData(X + Width - 1);
	GlcdWriteCmd(CASET);
	GlcdWriteData(Y);
	GlcdWriteData(Y + Height - 1);
	GlcdWriteCmd(RAMWR);
	WriteRomDataToLCD(Bitmap + 5, ByteCount);
	GlcdWriteCmd(NOOP);
	DeselectLCD();
}

static void SpiInit(void)
{
	// LCD piout setup
    	if( ioperm(BASEPORT, 3, 1) ) {
		perror( "ioperm" );
		exit( 1 );
	}
	
    	outb( 0x00, BASEPORT );        
	outb(( inb(BASEPORT) | GLCD_CS), BASEPORT );
        outb(( inb(BASEPORT) &~ GLCD_RESET), BASEPORT );
}

//
//  Write 1 byte
//
//
static void GlcdWriteCmd(char data)
{
    
        outb(( inb(BASEPORT) &~ GLCD_CS), BASEPORT );
        outb(( inb(BASEPORT) &~ SPI_SDO), BASEPORT );
        outb(( inb(BASEPORT) | SPI_SCK), BASEPORT );
        outb(( inb(BASEPORT) &~ SPI_SCK), BASEPORT );
	SpiByteSend(data);
}

static void WriteRomDataToLCD( char *RomData, unsigned int Count)
{	unsigned int i;
	
	for (i = 0; i < Count; i++)
	{
		GlcdWriteData(*RomData++);
	}
}

static void GlcdWriteData(char data)
{
        outb(( inb(BASEPORT) | SPI_SDO), BASEPORT );
        outb(( inb(BASEPORT) | SPI_SCK), BASEPORT );
        outb(( inb(BASEPORT) &~ SPI_SCK), BASEPORT );
	SpiByteSend(data);
}
// Msb first 
static void SpiByteSend (char spi_data){

        outb(( inb(BASEPORT) &~ SPI_SDO), BASEPORT );
        if (spi_data & 0x80) outb(( inb(BASEPORT) | SPI_SDO), BASEPORT );
        outb(( inb(BASEPORT) | SPI_SCK), BASEPORT ); outb(( inb(BASEPORT) &~ SPI_SCK), BASEPORT ); 
        
        outb(( inb(BASEPORT) &~ SPI_SDO), BASEPORT );
        if (spi_data & 0x40) outb(( inb(BASEPORT) | SPI_SDO), BASEPORT );
        outb(( inb(BASEPORT) | SPI_SCK), BASEPORT ); outb(( inb(BASEPORT) &~ SPI_SCK), BASEPORT ); 

        outb(( inb(BASEPORT) &~ SPI_SDO), BASEPORT );
        if (spi_data & 0x20) outb(( inb(BASEPORT) | SPI_SDO), BASEPORT );
        outb(( inb(BASEPORT) | SPI_SCK), BASEPORT ); outb(( inb(BASEPORT) &~ SPI_SCK), BASEPORT ); 
        
        outb(( inb(BASEPORT) &~ SPI_SDO), BASEPORT );
        if (spi_data & 0x10) outb(( inb(BASEPORT) | SPI_SDO), BASEPORT );
        outb(( inb(BASEPORT) | SPI_SCK), BASEPORT ); outb(( inb(BASEPORT) &~ SPI_SCK), BASEPORT ); 
        
        outb(( inb(BASEPORT) &~ SPI_SDO), BASEPORT );
        if (spi_data & 0x08) outb(( inb(BASEPORT) | SPI_SDO), BASEPORT );
        outb(( inb(BASEPORT) | SPI_SCK), BASEPORT ); outb(( inb(BASEPORT) &~ SPI_SCK), BASEPORT ); 
        
        outb(( inb(BASEPORT) &~ SPI_SDO), BASEPORT );
        if (spi_data & 0x04) outb(( inb(BASEPORT) | SPI_SDO), BASEPORT );
        outb(( inb(BASEPORT) | SPI_SCK), BASEPORT ); outb(( inb(BASEPORT) &~ SPI_SCK), BASEPORT ); 

        outb(( inb(BASEPORT) &~ SPI_SDO), BASEPORT );
        if (spi_data & 0x02) outb(( inb(BASEPORT) | SPI_SDO), BASEPORT );
        outb(( inb(BASEPORT) | SPI_SCK), BASEPORT ); outb(( inb(BASEPORT) &~ SPI_SCK), BASEPORT ); 
        
        outb(( inb(BASEPORT) &~ SPI_SDO), BASEPORT );
        if (spi_data & 0x01) outb(( inb(BASEPORT) | SPI_SDO), BASEPORT );
        outb(( inb(BASEPORT) | SPI_SCK), BASEPORT ); outb(( inb(BASEPORT) &~ SPI_SCK), BASEPORT ); 
}

static void DeselectLCD(void)
{
        outb(( inb(BASEPORT) | GLCD_CS), BASEPORT );
}