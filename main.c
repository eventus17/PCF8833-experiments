/*Copyright Adrian Przekwas
adrian.v.przekwas@gmail.com



0.5V/g
0g - 1.25V = 127
+1g = 1.75V = 178
-1g = 0.75V = 76
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
    if(!(PINB & _BV(PB0)))
    {
        _delay_ms(25);
        if(!(PINB & _BV(PB0)))
        {
            //do something
        }
    }
}

int main(void)
{	
    
        DDRB = 0x00;
        
        LCDDriverInitialize(LCD_COLOR_MODE_RGB8); 
	GlcdClear();
        
        SetScrolling();
        SetSep(0); //scrolling entry point
        adcInit();
        
        sei(); //enable interrupt
        
        uint8_t xpos1 = 64;
        uint8_t xpos2 = 64;
        uint8_t xpos1old = 64;
        uint8_t xpos2old = 64;
        uint8_t ypos = 0;
        char  buf5[5], buf3[3];
        float adcXf = 0.0;
        float adcYf = 0.0;
        uint8_t counter = 0;
        int angleX = 0;
        int angleY = 0;
        
        while(1)
        {
            //checkButtons();

            cli();
            xpos1 = adcX >> 1;
            xpos2 = adcY >> 1;
            if (counter >= 20)
            {
                if (ypos <= SPOS)
                {
                    DrawColumn_RGB8(ypos,RGB8_WHITE); //clear vertical line
                    //DrawLine_RGB8(ypos, xpos1old, ypos + 1, xpos1, RGB8_BLUE);
                    //DrawLine_RGB8(ypos, xpos2old, ypos + 1, xpos2, RGB8_RED);
                    PutPixel_RGB8(ypos, 128-xpos1, RGB8_BLUE); //draw data
                    PutPixel_RGB8(ypos, 128-xpos2, RGB8_RED);
                    ypos++;
                }
                SetSep(ypos);
                if (ypos > SPOS)  ypos = 0x00;
                
                counter = 0;
            }
            else
            {
                counter++;
            }
            
            DrawFilledRect_RGB8(xpos1old - 2, 128 - xpos2old - 2, xpos1old + 2, 128 - xpos2old + 2, RGB8_WHITE); //clear screen non-scrolling area, partial, only former cursor area
            //DrawCircle_RGB8(xpos1, xpos2, 2, RGB8_RED);
            DrawPointer5(xpos1, 128 - xpos2, RGB8_RED, RGB8_WHITE); //Y axis have to be mirrored
            
            DrawCircle_RGB8(64, 64, 12, RGB8_GREEN); 
            DrawCircle_RGB8(64, 64, 25, RGB8_GREEN);
            
            adcXf = (float)adcX * 0.0196f - 2.5f; //((ADC/255*2.5)-1.25)/0.5
            adcYf = (float)adcY * 0.0196f - 2.5f;
            
            //sprintf(buf,"%4u",adcX);
            sprintf(buf5,"%+1.2f",adcXf);
            DrawStr_8(buf5,100, 60, RGB8_BLUE, RGB8_WHITE);
            //sprintf(buf,"%4u",adcY);
            sprintf(buf5,"%+1.2f",adcYf);
            DrawStr_8(buf5, 100, 70, RGB8_RED, RGB8_WHITE);
        
            angleX = GetAngle(adcX);
            sprintf(buf3,"%+2d", angleX);
            DrawStr_8(buf3,100, 80, RGB8_BLUE, RGB8_WHITE);
        
            angleY = GetAngle(adcY);
            sprintf(buf3,"%+2d", angleY);
            DrawStr_8(buf3,100, 90, RGB8_RED, RGB8_WHITE);
            
            xpos1old = xpos1;
            xpos2old = xpos2;
            
            sei();
        }
        return 0;
}

