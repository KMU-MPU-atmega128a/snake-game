#include <iom128v.h>
#include "my128.h"
#include "lcd.h"
#include "intro.h"

Byte flag = 0;  // 0: default, pause / 1: playing
Byte snake = 5; // 뱀의 길이, 점수로 활용

Byte x_max = 50;
Byte y_max = 15;
Byte pixel_x = 0;   // 0~49, 가로 pixel (10칸*5pixel)
Byte pixel_y = 0;   // 0~15, 세로 pixel  (2칸*8pixel)

Byte head_x = 5;    // snake head x
Byte head_y = 0;    // snake head y
Byte tail_x = 0;    // snake tail x
Byte tail_y = 0;    // snake tail y

Byte dir_num = 0;
Byte dx[4] = {0x01, 0x02, 0x00, 0x01};
Byte dy[4] = {0x02, 0x01, 0x01, 0x00};
Byte nx, ny;

Byte InRange(Byte x, Byte y) {
    if (0 <= x && x < x_max && 0 <= y && y <= y_max) return 1;
    else return 0;
}

void snake_handler(void) {

}

#pragma interrupt_handler ext_int0_isr: iv_EXT_INT0

void ext_int0_isr(void) {

}

#pragma interrupt_handler ext_int1_isr: iv_EXT_INT1

void ext_int1_isr(void) {

}

#pragma interrupt_handler ext_int7_isr: iv_EXT_INT7

void ext_int7_isr(void) {
    flag ^= (1 << 0x01);    // game start, pause, resume button
}

void Interrupt_Init(void) {
    SREG |= 0x80;
    EIMSK |= (1 << INT1) | (1 << INT0);
    EICRA |= (1 << ISC11) | (1 << ISC01);
}

void main(void) {
    DDRB = 0xFF;    // PORTB set to output
    PORTB = 0xFF;   // all LEDs turned off
    DDRD = 0x00;    // PORTD set to input (buttons)

    PortInit();
    LCD_Init();
    Interrupt_Init();

    Cursor_Home();
    LCD_STR("Press PD7");
    LCD_pos(1,0);
    LCD_STR("to begin!");

    while (1) {
        if ((PIND & 0x80) == 0) {
            LCD_intro();    // display game intro
        }




    }

}

