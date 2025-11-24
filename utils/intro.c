#include "intro.h"

void LCD_intro(void) {
    LCD_Clear();
    Cursor_Home();
    LCD_STR("3...");
    LCD_delay(500);
    Cursor_Home();
    LCD_STR("2...");
    LCD_delay(500);
    Cursor_Home();
    LCD_STR("1...");
    LCD_delay(500);
    Cursor_Home();
    LCD_STR("GO!!");
    LCD_delay(500);
    LCD_Clear();

}
