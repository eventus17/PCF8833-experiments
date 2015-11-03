/*Copyright Adrian Przekwas
adrian.v.przekwas@gmail.com
*/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h> 
#include <stdio.h>
#include "LCDDriver.h"
#include "uart.h"

/* define CPU frequency in Mhz here if not defined in Makefile */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/********************************************************************************
Macros and Defines
********************************************************************************/

#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

int main(void)
{	
    
        //init the UART -- uart_init() is in uart.c
        usart_init ( MYUBRR );
        DDRB = 0x00;
        
        LCDDriverInitialize(LCD_COLOR_MODE_RGB8); 
	GlcdClear();
        
        char ch0, ch1;
        
        sei(); //enable interrupt
        
        while(1)
        {
            ch0 = usart_getchar();
            if (ch0 == 0x01)
            {
                ch1 = usart_getchar();
                GlcdWriteCmd(ch1);
            }
            else if (ch0 == 0x02)
            {
                ch1 = usart_getchar();
                GlcdWriteData(ch1);
            }
            else if (ch0 == 0x00)
            {
                DeselectLCD();
            }
                
        }
        return 0;
}

