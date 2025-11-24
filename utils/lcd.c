#include "lcd.h"

void LCD_delay(unsigned int ms) {   // LCD time delay function
    delay_ms(ms);
}

void PortInit(void) {       // PORT initiallization for LCD usage
    DDRA = 0xFF;    // PORTA set to output
    DDRG = 0x0F;    // PG~0 set to output
}

void LCD_Data(Byte ch) {    // input data to Data Register function
    LCD_CTRL |= (1 << LCD_RS);  // RS  = 1
    LCD_CTRL &= ~(1 << LCD_RW); // R/W = 0, registers set for data input
    LCD_CTRL |= (1 << LCD_EN);  // EN  = 1, LCD enabled
    delay_us(50);               // time delay

    LCD_WDATA = ch;             // data output
    delay_us(50);               // time delay

    LCD_CTRL &= ~(1 << LCD_EN); // EN  = 0, LCD disabled
}

void LCD_Comm(Byte ch) {    // input inst to Instruction Register function
    LCD_CTRL &= ~(1 << LCD_RS); // RS  = 0
    LCD_CTRL &= ~(1 << LCD_RW); // R/W = 0, registers set for inst input
    LCD_CTRL |= (1 << LCD_EN);  // EN  = 1, LCD enabled
    delay_us(50);               // time delay

    LCD_WINST = ch;             // inst input
    delay_us(50);               // time delay

    LCD_CTRL &= ~(1 << LCD_EN); // EN  = 0, LCD disabled
}

void LCD_Shift(char p) {    // shift LCD screen to left or right
    if (p == RIGHT) {
        LCD_Comm(0x1C);     // switch from A to C
        LCD_delay(1);       // time delay
    }

    if (p == LEFT) {
        LCD_Comm(0x18);
        LCD_delay(1);
    }
}

void LCD_CHAR(Byte c) {     // print one character
    LCD_delay(1);           // time delay (BF check)
    LCD_Data(c);            // transmit data to DDRAM
}

void LCD_STR(Byte *str) {   // print string
    while(*str != 0) {
        LCD_CHAR(*str);
        str++;
    }
}

void LCD_pos(unsigned char row, unsigned char col) { // select output position
    LCD_Comm(0x80 | (row * 0x40 + col));
}

void LCD_Clear(void) {      // clear LCD
    LCD_Comm(0x01);
    LCD_delay(2);
}

void LCD_Init(void) {       // initiallize LCD
    LCD_Comm(0x38);         // DDRAM, 8-bit data usage, use 2 columns
    LCD_delay(2);           // delay 2ms
    LCD_Comm(0x38);         // DDRAM, 8-bit data usage, use 2 columns
    LCD_delay(2);           // delay 2ms
    LCD_Comm(0x38);         // DDRAM, 8-bit data usage, use 2 columns
    LCD_delay(2);           // delay 2ms

    LCD_Comm(0x0C);         // display ON, cursor OFF, blink OFF
    LCD_delay(2);           // delay 2ms
    LCD_Comm(0x06);         // add 1 to address, move cursor to right
    LCD_delay(2);           // delay 2ms

    LCD_Clear();            // clear LCD
}

void Cursor_Home(void) {    // move cursor to (0, 0)
    LCD_Comm(0x02);         // move cursor to home
    LCD_delay(2);           // delay 2ms
}
