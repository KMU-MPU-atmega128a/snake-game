#ifndef __lcd_h
#define __lcd_h

#include <iom128v.h>
#include "my128.h"

#define LCD_WDATA   PORTA
#define LCD_WINST   PORTA
#define LCD_CTRL    PORTG
#define LCD_EN      0
#define LCD_RW      1
#define LCD_RS      2

#define On          1
#define OFF         0

#define RIGHT       1
#define LEFT        0

#define sbi(x, y)   (x |= (1 << y))     // set Bit Y of REG X
#define cbi(x, y)   (x &= ~(1 << y))    // clear Bit Y of REG X


void LCD_delay(unsigned int ms);        // LCD time delay function
void PortInit(void);            // PORT initiallization for LCD usage
void LCD_Data(Byte ch);         // input data to Data Register function
void LCD_Comm(Byte ch);         // input inst to Instruction Register function
void LCD_Shift(char p);         // shift LCD screen to left or right
void LCD_CHAR(Byte c);          // print one character
void LCD_STR(Byte *str);        // print string
void LCD_pos(unsigned char row, unsigned char col); // select output position
void LCD_Clear(void);           // clear LCD
void LCD_Init(void);            // initiallize LCD
void Cursor_Home(void);         // move cursor to (0, 0)

#endif
