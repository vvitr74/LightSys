/***************************************************************************
 * Original of this program created by unknown author. Fully
 * refactored, repaired and adapted to use in STM32F334 with HAL.
 ***************************************************************************
 * @file    ssd1306.c
 * @author  unknown author
 * @version V1.0
 * @date    05 - Mart - 2013
 * @brief   OLED display driver with I2C simulator.
 ******************************************************************************
 *  This software developed by engineer Viacheslav Azarov with free tools
 *  from STMicroelectronics on personal wish employer Danil Rogalev and saves
 *  all rights all authors and contributors.
 ******************************************************************************
 */

#include <ssd1306.h>

#define I2C_SIMULATOR_PORT			GPIOB
#define I2C_SIMULATOR_SDA			GPIO_PIN_9
#define I2C_SIMULATOR_SCL	  		GPIO_PIN_8
#define I2C_SIMULATOR_DELAY   		50

#define SSD1306_ADDRESS			    0x78
#define SSD1306_COMMAND				0x80
#define SSD1306_DATA			    0x40

#define SSD1306_DISPLAY_WIDTH      	128
#define SSD1306_DISPLAY_HEIGHT     	64
#define SSD1306_CURRY_MARKER		0xFF

// Buffer of character indexes (mirror of screen)
static uint8_t CursorCollumn, CursorRow;
static union {
    uint8_t Buffer [SSD1306_TEXT_HEIGHT * SSD1306_TEXT_WIDTH];
    uint8_t Map    [SSD1306_TEXT_HEIGHT] [SSD1306_TEXT_WIDTH];
} ScreenMirror;
static uint16_t CarryMarkers; // Bits represent string carry markers

static const uint8_t SmallFont5x7 [96][5] = {
	{0x00, 0x00, 0x00, 0x00, 0x00},// (space)
	{0x00, 0x00, 0x5F, 0x00, 0x00},// !
	{0x00, 0x07, 0x00, 0x07, 0x00},// "
	{0x14, 0x7F, 0x14, 0x7F, 0x14},// #
	{0x24, 0x2A, 0x7F, 0x2A, 0x12},// $
	{0x23, 0x13, 0x08, 0x64, 0x62},// %
	{0x36, 0x49, 0x55, 0x22, 0x50},// &
	{0x00, 0x05, 0x03, 0x00, 0x00},// '
	{0x00, 0x1C, 0x22, 0x41, 0x00},// (
	{0x00, 0x41, 0x22, 0x1C, 0x00},// )
	{0x08, 0x2A, 0x1C, 0x2A, 0x08},// *
	{0x08, 0x08, 0x3E, 0x08, 0x08},// +
	{0x00, 0x50, 0x30, 0x00, 0x00},// ,
	{0x08, 0x08, 0x08, 0x08, 0x08},// -
	{0x00, 0x30, 0x30, 0x00, 0x00},// .
	{0x20, 0x10, 0x08, 0x04, 0x02},// /
	{0x3E, 0x51, 0x49, 0x45, 0x3E},// 0
	{0x00, 0x42, 0x7F, 0x40, 0x00},// 1
	{0x42, 0x61, 0x51, 0x49, 0x46},// 2
	{0x21, 0x41, 0x45, 0x4B, 0x31},// 3
	{0x18, 0x14, 0x12, 0x7F, 0x10},// 4
	{0x27, 0x45, 0x45, 0x45, 0x39},// 5
	{0x3C, 0x4A, 0x49, 0x49, 0x30},// 6
	{0x01, 0x71, 0x09, 0x05, 0x03},// 7
	{0x36, 0x49, 0x49, 0x49, 0x36},// 8
	{0x06, 0x49, 0x49, 0x29, 0x1E},// 9
	{0x00, 0x36, 0x36, 0x00, 0x00},// :
	{0x00, 0x56, 0x36, 0x00, 0x00},// ;
	{0x00, 0x08, 0x14, 0x22, 0x41},// <
	{0x14, 0x14, 0x14, 0x14, 0x14},// =
	{0x41, 0x22, 0x14, 0x08, 0x00},// >
	{0x02, 0x01, 0x51, 0x09, 0x06},// ?
	{0x32, 0x49, 0x79, 0x41, 0x3E},// @
	{0x7E, 0x11, 0x11, 0x11, 0x7E},// A
	{0x7F, 0x49, 0x49, 0x49, 0x36},// B
	{0x3E, 0x41, 0x41, 0x41, 0x22},// C
	{0x7F, 0x41, 0x41, 0x22, 0x1C},// D
	{0x7F, 0x49, 0x49, 0x49, 0x41},// E
	{0x7F, 0x09, 0x09, 0x01, 0x01},// F
	{0x3E, 0x41, 0x41, 0x51, 0x32},// G
	{0x7F, 0x08, 0x08, 0x08, 0x7F},// H
	{0x00, 0x41, 0x7F, 0x41, 0x00},// I
	{0x20, 0x40, 0x41, 0x3F, 0x01},// J
	{0x7F, 0x08, 0x14, 0x22, 0x41},// K
	{0x7F, 0x40, 0x40, 0x40, 0x40},// L
	{0x7F, 0x02, 0x04, 0x02, 0x7F},// M
	{0x7F, 0x04, 0x08, 0x10, 0x7F},// N
	{0x3E, 0x41, 0x41, 0x41, 0x3E},// O
	{0x7F, 0x09, 0x09, 0x09, 0x06},// P
	{0x3E, 0x41, 0x51, 0x21, 0x5E},// Q
	{0x7F, 0x09, 0x19, 0x29, 0x46},// R
	{0x46, 0x49, 0x49, 0x49, 0x31},// S
	{0x01, 0x01, 0x7F, 0x01, 0x01},// T
	{0x3F, 0x40, 0x40, 0x40, 0x3F},// U
	{0x1F, 0x20, 0x40, 0x20, 0x1F},// V
	{0x7F, 0x20, 0x18, 0x20, 0x7F},// W
	{0x63, 0x14, 0x08, 0x14, 0x63},// X
	{0x03, 0x04, 0x78, 0x04, 0x03},// Y
	{0x61, 0x51, 0x49, 0x45, 0x43},// Z
	{0x00, 0x00, 0x7F, 0x41, 0x41},// [
	{0x02, 0x04, 0x08, 0x10, 0x20},// "\"
	{0x41, 0x41, 0x7F, 0x00, 0x00},// ]
	{0x04, 0x02, 0x01, 0x02, 0x04},// ^
	{0x40, 0x40, 0x40, 0x40, 0x40},// _
	{0x00, 0x01, 0x02, 0x04, 0x00},// `
	{0x20, 0x54, 0x54, 0x54, 0x78},// a
	{0x7F, 0x48, 0x44, 0x44, 0x38},// b
	{0x38, 0x44, 0x44, 0x44, 0x20},// c
	{0x38, 0x44, 0x44, 0x48, 0x7F},// d
	{0x38, 0x54, 0x54, 0x54, 0x18},// e
	{0x08, 0x7E, 0x09, 0x01, 0x02},// f
	{0x08, 0x14, 0x54, 0x54, 0x3C},// g
	{0x7F, 0x08, 0x04, 0x04, 0x78},// h
	{0x00, 0x44, 0x7D, 0x40, 0x00},// i
	{0x20, 0x40, 0x44, 0x3D, 0x00},// j
	{0x00, 0x7F, 0x10, 0x28, 0x44},// k
	{0x00, 0x41, 0x7F, 0x40, 0x00},// l
	{0x7C, 0x04, 0x18, 0x04, 0x78},// m
	{0x7C, 0x08, 0x04, 0x04, 0x78},// n
	{0x38, 0x44, 0x44, 0x44, 0x38},// o
	{0x7C, 0x14, 0x14, 0x14, 0x08},// p
	{0x08, 0x14, 0x14, 0x18, 0x7C},// q
	{0x7C, 0x08, 0x04, 0x04, 0x08},// r
	{0x48, 0x54, 0x54, 0x54, 0x20},// s
	{0x04, 0x3F, 0x44, 0x40, 0x20},// t
	{0x3C, 0x40, 0x40, 0x20, 0x7C},// u
	{0x1C, 0x20, 0x40, 0x20, 0x1C},// v
	{0x3C, 0x40, 0x30, 0x40, 0x3C},// w
	{0x44, 0x28, 0x10, 0x28, 0x44},// x
	{0x0C, 0x50, 0x50, 0x50, 0x3C},// y
	{0x44, 0x64, 0x54, 0x4C, 0x44},// z
	{0x00, 0x08, 0x36, 0x41, 0x00},// {
	{0x00, 0x00, 0x7F, 0x00, 0x00},// |
	{0x00, 0x41, 0x36, 0x08, 0x00},// }
	{0x02, 0x01, 0x02, 0x04, 0x02},// ~
	{0x7F, 0x41, 0x41, 0x41, 0x7F},// undefined symbols
};

void SSD1306_MspInit(void);
void SSD1306_Command(uint8_t data);
void SSD1306_DrawChar(uint8_t row, uint8_t col);

void SSD1306_Init(void)
{
	SSD1306_MspInit();

	SSD1306_Command(0xAE); //--turn off SSD1306 panel

	SSD1306_Command(0x20); //Set Memory Addressing Mode
	SSD1306_Command(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;
						   //10,Page Addressing Mode (RESET);11,Invalid

	SSD1306_Command(0xB0); //Set Page Start Address for Page Addressing Mode,0-7
	SSD1306_Command(0xC8); //Set COM Output Scan Direction
	SSD1306_Command(0x00); //---set low column address
	SSD1306_Command(0x10); //---set high column address
	SSD1306_Command(0x40); //--set start line address
	SSD1306_Command(0x81); //--set contrast control register
	SSD1306_Command(0xFF);

	//LCDI2C_WriteCommand(0xA3); //--Set No. of rows in top fixed area.
	//LCDI2C_WriteCommand(0x00); //--set normal display

	SSD1306_Command(0xA1); //--set segment re-map 0 to 127
	SSD1306_Command(0xA6); //--set normal display
	SSD1306_Command(0xA8); //--set multiplex ratio(1 to 64)
	SSD1306_Command(0x3F); //

	SSD1306_Command(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	SSD1306_Command(0xD3); //-set display offset
	SSD1306_Command(0x00); //-not offset
	SSD1306_Command(0xD5); //--set display clock divide ratio/oscillator frequency
	SSD1306_Command(0xF0); //--set divide ratio
	SSD1306_Command(0xD9); //--set pre-charge period
	SSD1306_Command(0x22); //
	SSD1306_Command(0xDA); //--set com pins hardware configuration
	SSD1306_Command(0x12);
	SSD1306_Command(0xDB); //--set vcomh
	SSD1306_Command(0x20); //0x20,0.77xVcc
	SSD1306_Command(0x8D); //--set DC-DC enable
	SSD1306_Command(0x14); //

	SSD1306_Command(0x2E); // stop scrolling

	SSD1306_Clear();	   // Clear buffer and display memory

	SSD1306_Command(0xAF); //--turn on SSD1306 panel
}

void SSD1306_MspInit(void)
{
 GPIO_InitTypeDef GPIO_InitStruct;

 __GPIOB_CLK_ENABLE();

GPIO_InitStruct.Pin = I2C_SIMULATOR_SDA| I2C_SIMULATOR_SCL;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
GPIO_InitStruct.Pull = GPIO_PULLUP;
GPIO_InitStruct.Speed= GPIO_SPEED_MEDIUM;
HAL_GPIO_Init(I2C_SIMULATOR_PORT, &GPIO_InitStruct);

}

void SCL0(void)
{
	HAL_GPIO_WritePin(I2C_SIMULATOR_PORT,I2C_SIMULATOR_SCL,GPIO_PIN_RESET);
}
void SCL1(void)
{
	HAL_GPIO_WritePin(I2C_SIMULATOR_PORT,I2C_SIMULATOR_SCL,GPIO_PIN_SET);
}
void SDA0(void)
{
	HAL_GPIO_WritePin(I2C_SIMULATOR_PORT,I2C_SIMULATOR_SDA,GPIO_PIN_RESET);
}
void SDA1(void)
{
	HAL_GPIO_WritePin(I2C_SIMULATOR_PORT,I2C_SIMULATOR_SDA,GPIO_PIN_SET);
}

void I2C_Delay(void)
{
	uint32_t cnt =  I2C_SIMULATOR_DELAY;
	while(cnt-- > 0){__asm("nop");}
}

void I2C_Start(void)
{
  SDA1();              
  I2C_Delay();
  SCL1();
  I2C_Delay();
  SDA0();              
  I2C_Delay();
  SCL0();
  I2C_Delay();
}

void I2C_Stop(void)
{
  SDA0();              
  I2C_Delay();
  SCL1();
  I2C_Delay();
  SDA1();              
  I2C_Delay();
}

uint8_t I2C_Transmit(unsigned char data)
{
  uint16_t x;  uint8_t res = 1;

  SCL0();
  for(x=8; x; x--)
  {
    if (data & 0x80) SDA1(); else  SDA0();
    SCL1();
    data <<= 1;
    I2C_Delay();	// Wait for transient of signal
    SCL0();			// Latch data in slave
    I2C_Delay();
  }
  SDA1(); SCL1();	// Acknowledge sequence
  I2C_Delay();
  res = HAL_GPIO_ReadPin(I2C_SIMULATOR_PORT, I2C_SIMULATOR_SDA);
  SCL0();
  I2C_Delay();
  return res;
}

void	SSD1306_Start(void)
{
	I2C_Start();
	I2C_Transmit(SSD1306_ADDRESS);
}
void	SSD1306_Stop(void)
{
	I2C_Stop();
}

void SSD1306_Command(uint8_t data)
{
	SSD1306_Start();
	I2C_Transmit(SSD1306_COMMAND);
	I2C_Transmit(data);
	SSD1306_Stop();
}

void SSD1306_Transmit(uint8_t data)
{
	SSD1306_Start();
	I2C_Transmit(SSD1306_DATA);
	I2C_Transmit(data);
	SSD1306_Stop();
}

void SSD1306_SetPointer(uint8_t row, uint8_t col)
{
	SSD1306_Command(0x00 + (6*col & 0x0F));
	SSD1306_Command(0x10 + (((6*col)>>4) & 0x0F));
	SSD1306_Command(0xB0 + row);
}


void SSD1306_SetPos(uint8_t row, uint8_t col)
{
	CursorRow     = (row < SSD1306_TEXT_HEIGHT-1)? row: SSD1306_TEXT_HEIGHT-1;
	CursorCollumn = (col < SSD1306_TEXT_WIDTH-1)?  col: SSD1306_TEXT_WIDTH-1;
	SSD1306_SetPointer(CursorRow, CursorCollumn);
}

uint8_t SSD1306_Row(void) {return CursorRow;}
uint8_t SSD1306_Col(void) {return CursorCollumn;}

void SSD1306_Clear(void)
{
	uint16_t pos, x; uint8_t row;
	CarryMarkers = 0;
	for (pos = 0; pos < SSD1306_TEXT_HEIGHT * SSD1306_TEXT_WIDTH;pos++) ScreenMirror.Buffer[pos] = 0;
	for (row = 0; row < SSD1306_TEXT_HEIGHT; row++)
	{
		SSD1306_SetPointer(row,0); for (x = 0; x < 128; x++) SSD1306_Transmit(0);
	}
	SSD1306_SetPos(0,0);
}

void SSD1306_ScroolUp(void)
{
   uint8_t row, col;
   uint16_t pos;
   uint16_t mask = 1;

// Shift lines in screen buffer
   for (pos = 0; pos < (SSD1306_TEXT_HEIGHT-1) * SSD1306_TEXT_WIDTH - 1; pos++)
		ScreenMirror.Buffer[pos] = ScreenMirror.Buffer[pos + SSD1306_TEXT_WIDTH];

   CarryMarkers >>= 1;

// Write spaces in screen buffer
   for (col = 0; col < SSD1306_TEXT_WIDTH; col++)
	     ScreenMirror.Map[SSD1306_TEXT_HEIGHT-1][col] = 0;

// Drawing lines from buffer
   for (row=0; row < SSD1306_TEXT_HEIGHT; row++)
   {
	   SSD1306_SetPointer(row,0);
	   for (col = 0; col < SSD1306_TEXT_WIDTH; col++) SSD1306_DrawChar(row,col);
	// Draw marker on last position
	   SSD1306_Transmit(0);
	   SSD1306_Transmit((CarryMarkers & mask)? SSD1306_CURRY_MARKER : 0);
	   mask <<= 1;
   }

// Shift cursor position
   SSD1306_SetPos((CursorRow==0)? 0: CursorRow-1,CursorCollumn);
}

void SSD1306_DrawChar(uint8_t row, uint8_t col)
{
	uint8_t c = ScreenMirror.Map[row][col], x;

	for (x = 0; x < 5; x++)	 // Five column in character
		SSD1306_Transmit(SmallFont5x7[c][x]);

	SSD1306_Transmit(0); 	// Space between characters
}

void SSD1306_PutChar(char c)
{
	// Carriage return control check
	if (c!='\n')
	{
		if ((c < 32) || (c > 127))   c  = 95;		// Special symbol for undefined characters
								else c -= 32;		// Convert to character index

		ScreenMirror.Map[CursorRow][CursorCollumn] = c;	// Table consist indexes in characters map

		SSD1306_DrawChar(CursorRow,CursorCollumn);	// Draw character from buffer on current position

		if (++CursorCollumn < SSD1306_TEXT_WIDTH)  return;

		//  Go to new line automatically
		CarryMarkers |= 1 << CursorRow;

		// Draw marker on last position
		SSD1306_Transmit(0);
		SSD1306_Transmit(SSD1306_CURRY_MARKER);
	}

	// Carriage return execute
	if (++CursorRow < SSD1306_TEXT_HEIGHT)
	{
		SSD1306_SetPos(CursorRow,0); return;
	}

	SSD1306_ScroolUp();
	SSD1306_SetPos(SSD1306_TEXT_HEIGHT - 1,0);
}
