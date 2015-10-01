/*Copyright Adrian Przekwas
adrian.v.przekwas@gmail.com
*/

//#define F_CPU 10000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h> 
#include <stdio.h>
#include "LCDDriver.h"

volatile uint8_t adcNum = 0; //start from pc0
volatile uint8_t adcX = 0;
volatile uint8_t adcY = 0;
volatile uint8_t baseTime = 1;
volatile uint8_t allowDrawing = 0;


ISR(ADC_vect)   //interrupt from ADC                     
{
    /*When ADCL is read, the ADFFPIC Data Register is not updated until ADCH is read. Consequently, if
the result is left adjusted and no more than 8-bit precision is required, it is sufficient to read
ADCH. Otherwise, ADCL must be read first, then ADCH.*/
    if (adcNum == 0)
    {
        adcX = ADCH;
        adcNum = 1;
    }
    else
    {
        adcY = ADCH;
        adcNum = 0;
    }
    ADMUX = adcNum | (1<<ADLAR); //set to AREF
    ADCSRA = _BV(ADEN)|_BV(ADIE)|_BV(ADSC)|_BV(ADPS2)|_BV(ADPS1); //set prescaler to 64 - set ADCSRA to re-enable ADC
}

ISR(TIMER2_COMPA_vect) //interrupt from timer2
{
    allowDrawing = 1;
}


void adcInit(void)
{
    ADMUX = adcNum | (1<<ADLAR); //set to AREF
    ADCSRA = _BV(ADEN)|_BV(ADIE)|_BV(ADSC)|_BV(ADPS2)|_BV(ADPS1); //set prescaler to 64
}

void checkButtons(void)
{
    char  buf2[3];
    if(!(PINB & _BV(PB0)))
    {
        _delay_ms(25);
        if(!(PINB & _BV(PB0)))
        {
            baseTime++;
            if (baseTime >= 255) baseTime = 1;
            OCR2A = baseTime;
            
            sprintf(buf2,"%3u",baseTime);
            DrawStr_8(buf2 ,110,60,RGB8_RED, RGB8_WHITE);
        }
    }
}


void timerInitialize(void)
{
    TCCR2B |= _BV(CS20) | _BV(CS21) | _BV(CS22); // Set 8 bit timer 2 with 1024 prescaler
    TCCR2A |= _BV(WGM21); // set timer CTC mode
    OCR2A = baseTime; // set demanded value on CTC
    TIMSK2 |= _BV(OCIE2A); // allow interrupt from CTC
}

int main(void)
{	
    
        DDRB = 0x00;
        
        LCDDriverInitialize(LCD_COLOR_MODE_RGB8); 
        timerInitialize();
	GlcdClear();
	
        DrawStr_8("TIM", 110, 70 , RGB8_RED, RGB8_WHITE);
        
        SetScrolling();
        SetSep(0); //scrolling entry point
        adcInit();
        
        sei(); //enable interrupt
        
        uint8_t xpos1 = 0;
        uint8_t xpos2 = 0;
        uint8_t ypos = 0;
        char  buf[3];
        sprintf(buf,"%3u",baseTime);
        DrawStr_8(buf ,110,60,RGB8_RED, RGB8_WHITE);
        
        while(1)
        {
            checkButtons();

            cli();
            if (allowDrawing)
            {
                if (ypos <= 0x63)
                {
                    xpos1 = adcX >> 1;
                    xpos2 = adcY >> 1;
                    DrawColumn_RGB8(ypos,RGB8_WHITE); //clear vertical line
                    PutPixel_RGB8(ypos, 128-xpos1, RGB8_BLUE); //draw data
                    PutPixel_RGB8(ypos, 128-xpos2, RGB8_RED);
                    ypos++;
                }
                SetSep(ypos);
                if (ypos > 0x63)  ypos = 0x00;
            }
            sei();
        }
        return 0;
}

