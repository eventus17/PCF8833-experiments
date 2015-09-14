/*Copyright Adrian Przekwas
adrian.v.przekwas@gmail.com
*/

//#define F_CPU 10000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h> 
#include "LCDDriver.h"




volatile uint8_t adcNum = 0; //start from pc0
volatile uint8_t adcData = 0;

ISR(ADC_vect)   //interrupt from ADC                     
{
    /*When ADCL is read, the ADC Data Register is not updated until ADCH is read. Consequently, if
the result is left adjusted and no more than 8-bit precision is required, it is sufficient to read
ADCH. Otherwise, ADCL must be read first, then ADCH.*/
    adcData = ADCH;
    
    ADMUX = adcNum | (1<<REFS0) | (1<<ADLAR); //REFS0 reference from V supply
    ADCSRA = _BV(ADEN)|_BV(ADIE)|_BV(ADSC)|_BV(ADPS2)|_BV(ADPS1); //set prescaler to 64 - set ADCSRA to re-enable ADC
}

adcInit(void)
{
    ADMUX = adcNum | (1<<REFS0) | (1<<ADLAR); //REFS0 reference from V supply
    //ADMUX  = (1<<REFS1) | (1<<REFS0) | (1<<ADLAR) | adcnum; //REFS1 and 2 means internal 2.56V as vref, when ADLAR=1 then ADCH is 8 most signifant bits
    ADCSRA = _BV(ADEN)|_BV(ADIE)|_BV(ADSC)|_BV(ADPS2)|_BV(ADPS1); //set prescaler to 64
}

int main(void)
{	
    LCDDriverInitialize(LCD_COLOR_MODE_RGB8);  
	GlcdClear();
	

	//DrawStr_8("ATMEGA8 6100 ",10,15,RGB8_GREEN, RGB8_WHITE);
	DrawChar_8('V', 110, 45 , RGB8_RED, RGB8_WHITE);
        
        //_delay_ms(2000);

        SetScrolling();
        SetSep(0); //scrolling entry point
        adcInit();
        
        sei(); //enable interrupt
        
        uint8_t xpos = 0;
        uint8_t ypos = 0;
        
        while(1)
        {
            if (ypos > 0x63) ypos = 0x00;
            xpos = adcData >> 2;
            DrawLine_RGB8(ypos,0,ypos,131,RGB8_WHITE); //clear horizontal line
            PutPixel_RGB8(ypos, xpos, RGB8_RED); //draw data
            SetSep(ypos);
            //_delay_ms(100);
            ypos++;
            
        }
        
        return 0;
}

