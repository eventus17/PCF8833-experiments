
/* ------------------------------------------------------------------------ */
/*							   Public constants				   				*/
/* ------------------------------------------------------------------------ */

//
// define the LCD's controller chip
//
#define LCD_CONTROLLER_PHILIPS_PCF8833 //Glcd sürücü entegre
#define GLCD_RESET_DDR		DDRD	// Glcd reset pini yönlendirme kaydedicisi
#define GLCD_RESET_PORT		PORTD	// Glcd reset pini portu
#define LCD_RESET PD0				// Glcd reset pini
#define GLCD_SPI_SDO_DDR		DDRD	// Glcd ser data pini yönlendirme kaydedicisi
#define GLCD_SPI_SDO_PORT		PORTD	// Glcd seri data pini portu
#define SPI_SDO PD1						// Glcd seri data pini
#define GLCD_SPI_SCK_DDR		DDRD	// Glcd saat sinyali pini yönlendirme kaydedicisi
#define GLCD_SPI_SCK_PORT		PORTD	// Glcd saat sinyali pini portu
#define SPI_SCK PD2						// Glcd saat sinyali pini
#define GLCD_CS_DDR		DDRD	// Glcd çip seçme pini yönlendirme kaydedicisi
#define GLCD_CS_PORT		PORTD // Glcd çip seçme pini portu
#define LCD_CS PD3						// Glcd çip seçme pini

//
// values for the Color Mode
//
#define LCD_COLOR_MODE_UNDEFINED 	0
#define LCD_COLOR_MODE_RGB12 		1
#define LCD_COLOR_MODE_RGB8 		2


#define LCD_WIDTH 132
#define LCD_HEIGHT 132


//
// drawing text modes
//
#define JUSTIFY_LEFT	0
#define JUSTIFY_CENTER	2
#define JUSTIFY_RIGHT	4


//
// RGB8 color definitions assuming the default color map
//
#define RGB8_WHITE 		0XFF
#define RGB8_BLACK 		0X00
#define RGB8_RED 		0XE0
#define RGB8_GREEN 		0X1C
#define RGB8_BLUE 		0X03
#define RGB8_YELLOW		0XFC
#define RGB8_PURPLE 	0XE3

#define RGB8_GRAY1 0b00100100			// darkest
#define RGB8_GRAY2 0b01001001
#define RGB8_GRAY3 0b01101101
#define RGB8_GRAY4 0b10010010
#define RGB8_GRAY5 0b10110110
#define RGB8_GRAY6 0b11011011			// lightest


#define RGB8_BLUE1 0b00000001			// darkest
#define RGB8_BLUE2 0b00000010	
#define RGB8_BLUE3 0b00000011	
#define RGB8_BLUE4 0b00100111
#define RGB8_BLUE5 0b01001011
#define RGB8_BLUE6 0b01101111
#define RGB8_BLUE7 0b10010011	
#define RGB8_BLUE8 0b10110111
#define RGB8_BLUE9 0b11011011			// lightest

#define RGB8_GREEN1 0b00001000			// darkest
#define RGB8_GREEN2 0b00001100	
#define RGB8_GREEN3 0b00010000	
#define RGB8_GREEN4 0b00110100
#define RGB8_GREEN5 0b01011000
#define RGB8_GREEN6 0b01111100
#define RGB8_GREEN7 0b01011101	
#define RGB8_GREEN8 0b10011110
#define RGB8_GREEN9 0b11011111			// lightest


//
// RGB12 color definitions assuming the default color map
//
#define RGB12_WHITE 	0XFFF
#define RGB12_BLACK 	0X000
#define RGB12_RED 		0XF00
#define RGB12_GREEN 	0X0F0
#define RGB12_BLUE 		0X00F
#define RGB12_YELLOW	0XFF2
#define RGB12_PURPLE 	0XE0E

#define SPOS 32

void LCDDriverInitialize(char ColorMode);
void LCDSelectColorMode(char, char *);
void GlcdClear(void);
void SetContrast(char);
void PutPixel_RGB8(char, char, char);
void DrawFilledRect_RGB8(char, char, char, char, char);
void DrawHorzGradientRGB8(char *, char, char, char, char);
void DrawLine_RGB8(int, int, int, int, char);
void DrawColumn_RGB8(int, char);
void DrawCircle_RGB8(int, int, int, char);
void DrawStr_8(const char *,char,char,char,char);
void DrawChar_8(char,char,char,char,char);
void DrawFilledRect_RGB212(char, char, char, char, int);
void DrawPixmap_RGB12( char *, char, char);
void SetScrolling(void);
void SetSep(uint8_t);
void DrawPointer5(char, char, char, char);
int GetAngle(uint8_t);


/* --------------------------- End of LCDDriver.h ------------------------- */
