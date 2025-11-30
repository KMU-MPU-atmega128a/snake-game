#include <iom128v.h>
#include "my128.h"
#include "lcd.h"
#include "intro.h"

Byte start_flag = 0;  // 0: default, pause / 1: playing
Byte snake = 3; // snake length, used as score

#define SNAKE_MAX_LENGTH 12

Byte dir = 0;

// dx dy for snake movement: Right, Down, Left, Up
Byte dx[4] = {2, 1, 0, 1};
Byte dy[4] = {1, 0, 1, 2};

// [MAX_SIZE]: 16x2 CLCD, 5x8 pixel
#define X_MAX 80
#define Y_MAX 16

// pixel 단위 좌표 (0~79, 0~15)
Byte x[X_MAX] = {0, };
Byte y[Y_MAX] = {0, };
// 먹이 좌표
Byte food_x;
Byte food_y;

// CLCD상의 좌표 (0~1, 0~15)
Byte x_LCD[X_MAX] = {0, };
Byte y_LCD[Y_MAX] = {0, };

// CLCD 한 칸에서의 pixel 단위 좌표 (0~4, 0~7)
Byte x_pixel[X_MAX] = {0, };
Byte y_pixel[Y_MAX] = {0, };


unsigned int cnt = 0;

// snake 속도 조절
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

unsigned int rand = 0;

#pragma interrupt_handler timer2_ovf_isr: iv_TIM2_OVF
void timer2_ovf_isr(void) {
    TCNT2 = 56;
    rand++;
}

void Init_Timer2(void) {
    TCCR2 |= (1 << CS21);   // 8분주
    TIMSK |= (1 << TOIE2);
    TCNT2 = 56;
}

void spawn_food(void) {
    Byte rand_x, rand_y;
    Byte is_on_snake = 0;

    do {
        is_on_snake = 0;

        rand_x = rand % X_MAX;
        rand_y = rand % Y_MAX;

        for (Byte i = 0; i < snake; i++) {
            if (x[i] == rand_x && y[i] == rand_y) {
                is_on_snake = 1;
                break;
            }
        }
    } while (is_on_snake);  // 안 겹칠 때까지 반복

    // food 위치 확정
    food_x = rand_x;
    food_y = rand_y;
}

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

    Byte snake_pos_x[8] = {0, };
    Byte snake_pos_y[8] = {0, };

    snake_pos_x[0] = x_LCD[0];
    snake_pos_y[0] = y_LCD[0];

    Byte CGRAM_data[64] = {0, };

    for (Byte i = 0; i < snake; i++) {
        Byte slot_found = 0;
        for (Byte j = 0; j < CGRAM_size; j++) {
            if (snake_pos_x[j] == x_LCD[i] && snake_pos_y[j] == y_LCD[i]) {   // 이미 그린 칸이 있는 좌표인 경우
                CGRAM_num = j;
                slot_found = 1;
                break;
            }
        }
        if (slot_found == 0) { // 새로운 칸인 경우
            snake_pos_x[CGRAM_size] = x_LCD[i];
            snake_pos_y[CGRAM_size] = y_LCD[i];
            CGRAM_num = CGRAM_size;
            CGRAM_size++;
        }

        CGRAM_data[CGRAM_num * 8 + y_pixel[i]] |= (0x10 >> x_pixel[i]); // 해당 칸, 행, 열에 그릴 그림 저장 (OR)

    }

    Byte food_LCD_x = food_y / 0x08;
    Byte food_LCD_y = food_x / 0x05;
    Byte food_pixel_x = food_x % 0x05;
    Byte food_pixel_y = food_y % 0x08;

    Byte food_CGRAM_num = 0;
    Byte food_slot_found = 0;

    for (Byte i = 0; i < CGRAM_size; i++) {
        if (snake_pos_x[i] == food_LCD_x && snake_pos_y[i] == food_LCD_y) {
            food_CGRAM_num = i;
            food_slot_found = 1;
            break;
        }
    }

    if (food_slot_found == 0) {
        food_CGRAM_num = CGRAM_size;
        snake_pos_x[food_CGRAM_num] = food_LCD_x;
        snake_pos_y[food_CGRAM_num] = food_LCD_y;
        CGRAM_size++;
    }

    CGRAM_data[food_CGRAM_num * 8 + food_pixel_y] |= (0x10 >> food_pixel_x);    // CGRAM에 먹이 그리기

    // 저장한 56개의 줄 전부 그리기
    LCD_Comm(0x40);
    for (Byte i = 0; i < 64; i++) {
        LCD_Data(CGRAM_data[i]);
    }

    LCD_Clear();
    for (Byte i = 0; i < CGRAM_size; i++) {
        LCD_pos(snake_pos_x[i], snake_pos_y[i]);
        LCD_Data(0x00 + i);
    }
}

void Snake_moveset(void) {
    Byte nx = x[0] + dx[dir] - 1;
    Byte ny = y[0] + dy[dir] - 1;

    // 좌우 경계 확인
    if (nx == 0xFF) nx = X_MAX - 1;
    else if (nx >= X_MAX) nx = 0;
    // 상하 경계 확인
    if (ny == 0xFF) ny = Y_MAX - 1;
    else if (ny >= Y_MAX) ny = 0;

    // 꼬리 좌표 임시로 저장
    Byte tail_x = x[snake - 1];
    Byte tail_y = y[snake - 1];

    for (Byte i = snake - 1; i >= 1; i--) {
        x[i] = x[i - 1];
        y[i] = y[i - 1];
    }

    x[0] = nx;
    y[0] = ny;

    // 먹이 먹었는지 확인
    if (x[0] == food_x && y[0] == food_y) {
        if (snake < SNAKE_MAX_LENGTH) {
            x[snake] = tail_x;
            y[snake] = tail_y;
            snake++;
        }
        spawn_food();
    }
}

void Snake_Init(void) {
    Byte x_tmp = 0x19;
    for (Byte i = 0; i < snake; i++) {
        x[i] = x_tmp--;
        y[i] = 0x07;
    }
    Snake_coords();
    Snake_generator();
    spawn_food();
}

void LED_blink(void) {  // LED blinker for invalid inputs
    for (Byte i = 0; i < 2; i++) {
        PORTB = 0x00;
        delay_ms(50);
        PORTB = 0xFF;
        delay_ms(50);
    }
}

#pragma interrupt_handler ext_int0_isr: iv_EXT_INT0
void ext_int0_isr(void) {   // dir: up
    if (dir != 3) dir = 1;
    else LED_blink();
}

#pragma interrupt_handler ext_int1_isr: iv_EXT_INT1
void ext_int1_isr(void) {   // dir: down
    if (dir != 1) dir = 3;
    else LED_blink();
}

#pragma interrupt_handler ext_int2_isr: iv_EXT_INT2
void ext_int2_isr(void) {   // dir: left
    if (dir != 0) dir = 2;
    else LED_blink();
}

#pragma interrupt_handler ext_int3_isr: iv_EXT_INT3
void ext_int3_isr(void) {   // dir: right
    if (dir != 2) dir = 0;
    else LED_blink();
}

void Interrupt_Init(void) {
    EIMSK |= (1 << INT0) | (1 << INT1) | (1 << INT2) | (1 << INT3); // allow INT0~3
    EICRA |= (1 << ISC01) | (1 << ISC11) | (1 << ISC21) | (1 << ISC31); // trigger
    SREG |= 0x80;
}

void main(void) {
    DDRB = 0xFF;    // PORTB set to output (LED)
    PORTB = 0xFF;   // all LEDs turned off

    DDRD = 0x00;    // PORTD set to input (buttons)

    PortInit();
    LCD_Init();
    Interrupt_Init();
    Init_Timer0();
    Init_Timer2();

    Cursor_Home();

    Snake_Init();

    while (1) {
        if (cnt > 500) {
            cnt = 0;
            Snake_moveset();
            Snake_coords();
            Snake_generator();
        }

    }

}
