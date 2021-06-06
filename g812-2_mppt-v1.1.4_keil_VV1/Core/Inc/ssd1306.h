/***************************************************************************
 * Original of this program created by unknown author. Fully
 * refactored, repaired and adapted to use in STM32F334 with HAL.
 ***************************************************************************
 * @file    ssd1306.h
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

#ifndef __SSD1306
#define __SSD1306
#include "stm32f3xx.h"

#define SSD1306_TEXT_WIDTH		21
#define SSD1306_TEXT_HEIGHT		8

// Initialize driver
void SSD1306_Init(void);

// Simplest ASCII terminal functions
void SSD1306_Clear(void);
void SSD1306_ScrollUp(void);

void SSD1306_SetPos(uint8_t row, uint8_t col);
uint8_t SSD1306_Row(void);
uint8_t SSD1306_Col(void);

// Displayed characters (space)..'~' and execute '\n'
void SSD1306_PutChar(char c);



#endif
