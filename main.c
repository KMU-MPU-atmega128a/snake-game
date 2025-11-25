#include <iom128v.h>
#include "my128.h"
#include "lcd.h"
#include "intro.h"

Byte start_flag = 0;  // 0: default, pause / 1: playing
Byte snake = 5; // snake length, used as score

Byte dir = 0;

// dx dy for snake movement: Right, Down, Left, Up
Byte dx[4] = {2, 1, 0, 1};
Byte dy[4] = {1, 0, 1, 2};

#define X_MAX 50
#define Y_MAX 16

Byte x[X_MAX] = {0, };
Byte y[Y_MAX] = {0, };
Byte x_LCD[X_MAX] = {0, };
Byte y_LCD[Y_MAX] = {0, };
Byte x_pixel[X_MAX] = {0, };
Byte y_pixel[Y_MAX] = {0, };

void Snake_coords(void) {
    for (Byte i = 0; i < snake; i++) {
        x_LCD[i] = y[i] / 0x08;
        y_LCD[i] = x[i] / 0x05;
        x_pixel[i] = x[i] % 0x05;
        y_pixel[i] = y[i] % 0x08;
    }
}

void Snake_generator(void) {
    Byte CGRAM_num = 0;
    Byte CGRAM_size = 1;

    Byte snake_pos_x[7] = {0, };
    Byte snake_pos_y[7] = {0, };

    snake_pos_x[0] = x_LCD[0];
    snake_pos_y[0] = y_LCD[0];

    Byte CGRAM_data[56] = {0, };

    for (Byte i = 0; i < snake; i++) {
        Byte tmp_flag = 0;
        for (Byte j = 0; j < CGRAM_size; j++) {
            if (snake_pos_x[j] == x_LCD[i] & snake_pos_y[j] == y_LCD[i]) {   // 이미 그린 칸이 있는 좌표인 경우
                CGRAM_num = j;
                tmp_flag = 1;
                continue;
            }
        }
        if (tmp_flag == 0) { // 새로운 칸인 경우
            snake_pos_x[CGRAM_size] = x_LCD[i];
            snake_pos_y[CGRAM_size] = y_LCD[i];
            CGRAM_num = CGRAM_size;
            CGRAM_size++;
        }

        CGRAM_data[CGRAM_num * 8 + y_pixel[i]] |= (0x10 >> x_pixel[i]); // 해당 칸, 행, 열에 그릴 그림 저장 (OR)

    }
    // 저장한 56개의 줄 전부 그리기
    LCD_Comm(0x40);
    for (Byte i = 0; i < 56; i++) {
        LCD_Data(CGRAM_data[i]);
        LCD_delay(5);
    }

    LCD_Clear();
    for (Byte i = 0; i < CGRAM_size; i++) {
        LCD_pos(snake_pos_x[i], snake_pos_y[i]);
        LCD_Data(0x00 + i);
        LCD_delay(5);
    }
}

void Snake_moveset(void) {
    for (Byte i = snake - 1; i >= 1; i--) {
        x[i] = x[i - 1];
        y[i] = y[i - 1];
    }
    x[0] += (dx[dir] - 1);
    y[0] += (dy[dir] - 1);
}

void Snake_Init(void) {
    Byte x_tmp = 0x19;
    for (Byte i = 0; i < 6; i++) {
        x[i] = x_tmp--;
        y[i] = 0x07;
    }
    Snake_coords();
    Snake_generator();
}

#pragma interrupt_handler ext_int0_isr: iv_EXT_INT0
void ext_int0_isr(void) {   // dir: up
    dir = 1;
    Snake_moveset();
    Snake_coords();
    Snake_generator();
}

#pragma interrupt_handler ext_int1_isr: iv_EXT_INT1
void ext_int1_isr(void) {   // dir: down
    dir = 3;
    Snake_moveset();
    Snake_coords();
    Snake_generator();
}

#pragma interrupt_handler ext_int2_isr: iv_EXT_INT2
void ext_int2_isr(void) {   // dir: left
    dir = 2;
    Snake_moveset();
    Snake_coords();
    Snake_generator();
}

#pragma interrupt_handler ext_int3_isr: iv_EXT_INT3
void ext_int3_isr(void) {   // dir: right
    dir = 0;
    Snake_moveset();
    Snake_coords();
    Snake_generator();
}

void Interrupt_Init(void) {
    EIMSK |= (1 << INT0) | (1 << INT1) | (1 << INT2) | (1 << INT3); // allow INT0~3
    EICRA |= (1 << ISC01) | (1 << ISC11) | (1 << ISC21) | (1 << ISC31); // trigger
    SREG |= 0x80;
}

// 최종 시연에만 타이머 사용
/*
unsigned int cnt;

#pragma interrupt_handler timer0_ovf_isr: iv_TIM0_OVF
void timer0_ovf_isr(void) {
    TCNT0 = 6;
    cnt++;
}

void Init_Timer0(void) {
    TCCR0 |= (1 << CS01) | (1 << CS00);
    TCCR0 &= ~(1 << CS02);
    TIMSK |= (1 << TOIE0);
    TCNT0 = 6;
}
*/

void main(void) {
    DDRB = 0xFF;    // PORTB set to output (LED)
    PORTB = 0xFF;   // all LEDs turned off

    DDRD = 0x00;    // PORTD set to input (buttons)

    PortInit();
    LCD_Init();
    Interrupt_Init();
    // Init_Timer0();

    Cursor_Home();

    Snake_Init();

    while (1) {

    }

    // 최종 시연에만 타이머 사용
    /*
    while (1) {
        if (cnt > 2000) {
            cnt = 0;
            Snake_moveset();
            Snake_coords();
            Snake_generator();
        }

    }
    */
}
