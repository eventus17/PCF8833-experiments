//#define F_CPU 10000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "LCDDriver.h"



int main(void)
{	
    LCDDriverInitialize(LCD_COLOR_MODE_RGB8);  
	GlcdClear();
	

	DrawStr_8("ERHAN YILMAZ",35,5,RGB8_BLUE, RGB8_WHITE);
	DrawStr_8("ATMEGA8 6100 ",10,15,RGB8_GREEN, RGB8_WHITE);
	DrawChar_8('R', 87, 15, RGB8_GREEN, RGB8_WHITE);
	DrawChar_8('E', 93, 15 , RGB8_BLACK, RGB8_WHITE);
	DrawChar_8('N', 99, 15 , RGB8_YELLOW, RGB8_WHITE);
	DrawChar_8('K', 105, 15 , RGB8_BLUE, RGB8_WHITE);
	DrawChar_8('L', 111, 15 , RGB8_RED, RGB8_WHITE);
	DrawChar_8('I', 117, 15 , RGB8_PURPLE, RGB8_WHITE);
	DrawStr_8("EKRAN DENEMESI",30,25,RGB8_RED, RGB8_WHITE);
	DrawFilledRect_RGB8(38, 50, 85,110,RGB8_BLUE);
	DrawCircle_RGB8(61, 80, 40,RGB8_RED);
	DrawLine_RGB8(10,35,120,35,RGB8_BLUE);

        return 0;
}

