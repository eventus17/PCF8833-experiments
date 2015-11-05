/*Copyright Adrian Przekwas
adrian.v.przekwas@gmail.com
*/

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "LCDDriver.h"


int main(void)
{	
        LCDDriverInitialize(LCD_COLOR_MODE_RGB8); 
	GlcdClear();
        usleep( 10000 );
	
        DrawStr_8("TIM", 110, 70 , RGB8_RED, RGB8_WHITE);
        DrawStr_8("OFF", 110, 90 , RGB8_BLUE, RGB8_WHITE);
        
        //SetScrolling();
        //SetSep(0); //scrolling entry point
        int i = 1;
        while(1)
        {
            DrawCircle_RGB8(64, 64, i, RGB8_RED);
            i++;
            if (i>50)
            {
                i = 1;
                GlcdClear();
            }
            //usleep( 10000 );
            
        }
        
        
        return 0;
}

