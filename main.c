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
volatile uint8_t adcData = 0;
volatile uint8_t baseTime = 1;
volatile uint8_t trigVal = 0;
volatile uint8_t trigEnab = 0;
volatile uint8_t allowDrawing = 0;


ISR(ADC_vect)   //interrupt from ADC                     
{
    /*When ADCL is read, the ADFFPIC Data Register is not updated until ADCH is read. Consequently, if
the result is left adjusted and no more than 8-bit precision is required, it is sufficient to read
ADCH. Otherwise, ADCL must be read first, then ADCH.*/
    adcData = ADCH;
    
    ADMUX = adcNum | (1<<REFS0) | (1<<ADLAR); //REFS0 reference from V supply
    ADCSRA = _BV(ADEN)|_BV(ADIE)|_BV(ADSC)|_BV(ADPS2)|_BV(ADPS1); //set prescaler to 64 - set ADCSRA to re-enable ADC
}

ISR(TIMER2_COMPA_vect) //interrupt from timer2
{
    allowDrawing = 1;
}


void adcInit(void)
{
    ADMUX = adcNum | (1<<REFS0) | (1<<ADLAR); //REFS0 reference from V supply
    //ADMUX  = (1<<REFS1) | (1<<REFS0) | (1<<ADLAR) | adcnum; //REFS1 and 2 means internal 2.56V as vref, when ADLAR=1 then ADCH is 8 most signifant bits
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
    if(!(PINB & _BV(PB1)))
    {
        _delay_ms(25);
        if(!(PINB & _BV(PB1)))
        {
            trigVal++;
            if (trigVal >= 255) trigVal = 1;
            
            char  buf2[3];
            sprintf(buf2,"%3u",trigVal);
            DrawStr_8(buf2,110,80,RGB8_BLUE, RGB8_WHITE);
        }
    }
    if(!(PINB & _BV(PB2)))
    {
        _delay_ms(500);
        if(!(PINB & _BV(PB2)))
        {
            if (!trigEnab)
            {
                DrawStr_8("TRG", 110, 90, RGB8_BLUE, RGB8_WHITE);
                trigEnab = 1;
            }
            else
            {
                DrawStr_8("OFF", 110, 90, RGB8_BLUE, RGB8_WHITE);
                trigEnab = 0;    
            }
        }
    }
}

uint8_t checkTriggering(uint8_t oldVal, uint8_t newVal)
{
    if (!trigEnab) return 1; 
    if (trigEnab == 1) //TODO falling, rising etc.
    {
        if ((trigVal > oldVal) && (trigVal <= newVal))
        {
            return 1;
        }
    }
    return 0;
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
        DrawStr_8("OFF", 110, 90 , RGB8_BLUE, RGB8_WHITE);
        
        SetScrolling();
        SetSep(0); //scrolling entry point
        adcInit();
        
        sei(); //enable interrupt
        
        uint8_t xpos = 0;
        uint8_t ypos = 0;
        uint8_t oldVal = 0;
        char  buf[3];
        sprintf(buf,"%3u",baseTime);
        DrawStr_8(buf ,110,60,RGB8_RED, RGB8_WHITE);
        
        while(1)
        {
            checkButtons();
            if (allowDrawing)
            {
                cli();
                if (ypos <= 0x63)
                {
                    xpos = adcData >> 1;
                    DrawColumn_RGB8(ypos,RGB8_WHITE); //clear vertical line
                    PutPixel_RGB8(ypos, 128-xpos, RGB8_BLUE); //draw data
                    ypos++;
                }
                if (checkTriggering(oldVal, adcData))
                {
                    SetSep(ypos);
                }
                if ((ypos > 0x63) && (checkTriggering(oldVal, adcData)))  ypos = 0x00;
                oldVal = adcData;
                allowDrawing = 0;
                sei();
            }
        }
        
        return 0;
}

