#include <iom128v.h>
#include "my128.h"
#include "lcd.h"

Byte data = 0;
Byte rxdata = 0;

// 수신 완료 인터럽트
#pragma interrupt_handler usart0_receive:iv_USART0_RXC
void usart0_receive(void) {
    rxdata = UDR0;
}

// asynchronous, x1, baud rate = 9,600bps, 8bit data, even parity, 1bit stop bit
void Init_USART0(void) {
    UCSR0A = 0x00; // 1x transfer rate

    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
    // allow receive/transmit (RXEN0,TXEN0 = 1)

    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    // asyncrhonous (UMSEL0 = 0), even parity (UPM01,UPM00 = 10)
    // 1bit stop bit (USBS0 = 0), 8bit data (UCSZ02~00 = 011)s

    UBRR0H = 0x00;
    UBRR0L = 0x67;
    // baud rate = 9,600 bps

    SREG |= 0x80;
}

void USART0_Transmit(Byte data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

Byte start_flag = 0;  // 0: default, pause / 1: playing
Byte snake = 3; // snake length, used as score
Byte food = 0;

#define SNAKE_MAX_LENGTH 13

Byte dir = 0;

// dx dy for snake movement: Right, Down, Left, Up
Byte dx[4] = {2, 1, 0, 1};
Byte dy[4] = {1, 0, 1, 2};

// [MAX_SIZE]: 16x2 CLCD, 5x8 pixel
#define X_MAX 80
#define Y_MAX 16

// 난이도 설정
#define DIFF_NORMAL 1
#define DIFF_HARD   2
Byte diff = DIFF_NORMAL;

// 일정 이동 횟수 후 속도 상승
// NORMAL: 0.8s -> 0.5s (cnt > 1600 -> 1000)
// HARD  : 0.3s -> 0.1s (cnt >  600 -> 200)
#define NORMAL_SLOW 1600
#define NORMAL_FAST 1000
#define HARD_SLOW    600
#define HARD_FAST    200
unsigned int move_interval = NORMAL_SLOW;

// pixel 단위 좌표 (0~79, 0~15)
Byte x[SNAKE_MAX_LENGTH] = {0, };
Byte y[SNAKE_MAX_LENGTH] = {0, };
// CLCD에서 뱀의 LCD 칸 좌표 (0~1, 0~15)
Byte x_LCD[X_MAX] = {0, };
Byte y_LCD[Y_MAX] = {0, };
// CLCD에서 뱀의 pixel 단위 좌표 (0~4, 0~7)
Byte x_pixel[X_MAX] = {0, };
Byte y_pixel[Y_MAX] = {0, };

// 먹이 좌표
Byte food_x;
Byte food_y;

volatile unsigned int cnt = 0;

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

// 난수 발생기로 TCNT2 사용
volatile unsigned int rand = 0;

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


// 장애물 좌표 생성
#define MAX_OBSTACLES 10
Byte obstacle_count = 0;

void Obstacle_Init(void) {
    // NORMAL: 5개, HARD: 10개
    obstacle_count = (diff == DIFF_HARD) ? MAX_OBSTACLES : 5;
    LCD_pos(0,2);  LCD_Data('X');
    LCD_pos(1,4);  LCD_Data('X');
    LCD_pos(1,8);  LCD_Data('X');
    LCD_pos(0,10); LCD_Data('X');
    LCD_pos(1,14); LCD_Data('X');

    if (diff == DIFF_HARD) {
        LCD_pos(0,1);  LCD_Data('X');
        LCD_pos(0,6);  LCD_Data('X');
        LCD_pos(1,12); LCD_Data('X');
        LCD_pos(1,13); LCD_Data('X');
        LCD_pos(1,15); LCD_Data('X');
    }
}

Byte is_obstacle(Byte x, Byte y) {
    if ((x == 0 && y == 2) || (x == 1 && y == 4) || (x == 1 && y == 8) ||
        (x == 0 && y == 10) || (x == 1 && y == 14) ) {
        return 1;
    }
    // hard: 10 obstacles
    if (diff == DIFF_HARD) {
        if ((x == 0 && y == 1) || (x == 0 && y == 6) || (x == 1 && y == 12) ||
            (x == 1 && y == 15) || (x == 1 && y == 13) ) {
            return 1;
        }
    }

    return 0; // 장애물 없음
}


void spawn_food(void) {
    Byte rand_pixel_x, rand_pixel_y;
    Byte error_flag = 0;

    do {
        error_flag = 0;

        rand_pixel_x = rand % X_MAX;
        rand_pixel_y = rand % Y_MAX;

        for (Byte i = 0; i < snake; i++) {
            if (x[i] == rand_pixel_x && y[i] == rand_pixel_y) {
                error_flag = 1;
                break;
            }
        }

        if (error_flag == 0) {
            Byte rand_LCD_x = rand_pixel_y / 8;
            Byte rand_LCD_y = rand_pixel_x / 5;
            if (is_obstacle(rand_LCD_x, rand_LCD_y)) {  // 장애물 확인
                error_flag = 1;
            }
        }

    } while (error_flag);  // 안 겹칠 때까지 반복

    // food 위치 확정
    food_x = rand_pixel_x;
    food_y = rand_pixel_y;
}

void Snake_generator(void) {
    for (Byte i = 0; i < snake; i++) {
        x_LCD[i] = y[i] / 0x08;
        y_LCD[i] = x[i] / 0x05;
        x_pixel[i] = x[i] % 0x05;
        y_pixel[i] = y[i] % 0x08;
    }

    Byte CGRAM_num = 0;
    Byte CGRAM_size = 1;

    Byte snake_pos_x[8] = {0, };    // 사용할 CLCD 칸 좌표 (0~1) , CGRAM은 8칸
    Byte snake_pos_y[8] = {0, };    // 사용할 CLCD 칸 좌표 (0~15), CGRAM은 8칸

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

        if (slot_found == 0) { // 새로운 칸에 그려야 하는 경우
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

    for (Byte i = 0; i < CGRAM_size; i++) { // 이미 그린 칸에 먹이가 그려질 경우
        if (snake_pos_x[i] == food_LCD_x && snake_pos_y[i] == food_LCD_y) {
            food_CGRAM_num = i;
            food_slot_found = 1;
            break;
        }
    }

    if (food_slot_found == 0 && CGRAM_size < 8) { // 새로운 CGRAM 칸에 그려질 경우
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

    Obstacle_Init();

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

    // 자기 몸과 충돌 검사
    for (Byte i = 1; i < snake; i++) {
        if (nx == x[i] && ny == y[i]) {
            start_flag = 0;
            return;
        }
    }

    Byte head_x = ny / 0x08;    // 0~15 -> 0~1
    Byte head_y = nx / 0x05;    // 0~79 -> 0~15

    // 장애물 충돌 검사
    if (is_obstacle(head_x, head_y)) {
        start_flag = 0;
        return;
    }

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
        food++;
        spawn_food();
    }
}

void Snake_Init(void) {
    Byte x_tmp = 0x19;
    snake = 3;
    food  = 0;
    for (Byte i = 0; i < snake; i++) {
        x[i] = x_tmp--;
        y[i] = 0x07;
    }

    Obstacle_Init();
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
    EICRA |= (1 << ISC01) | (1 << ISC11) | (1 << ISC21) | (1 << ISC31); // falling edge trigger
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
    Init_USART0();

    Cursor_Home();

    Snake_Init();
    start_flag = 0;

    while (1) {
        Byte received_diff = rxdata;

        if (start_flag == 0) {
            if (received_diff == '1') {
                diff = DIFF_NORMAL;
                Snake_Init();
                start_flag = 1;
            }
            else if (received_diff == '2') {
                diff = DIFF_HARD;
                Snake_Init();
                start_flag = 1;
            }
        }

        if (start_flag == 1) {
            // 뱀 속도 조절
            if (food >= 5) {
                USART0_Transmit('F');   // send 'F' as in fever time

                if (diff == DIFF_NORMAL) move_interval = NORMAL_FAST;
                else                     move_interval = HARD_FAST;
            }
            else {
                if (diff == DIFF_NORMAL) move_interval = NORMAL_SLOW;
                else                     move_interval = HARD_SLOW;
            }

            if (cnt > move_interval) {
                cnt = 0;
                Snake_moveset();    // 뱀 이동

                if (food == 10) {   // 먹이 10개 먹어서 game clear
                    USART0_Transmit('C');
                    start_flag = 0;
                }


                if (start_flag == 0) {  // 벽이나 자신의 몸에 부딪혀서 game over
                    PORTB = 0x00;
                    // 게임 종료 신호 전송 (END)
                    USART0_Transmit('L');

                    // 게임 재시작을 위해 rxdata 0으로 초기화해 난이도 남아있는 걸 방지
                    rxdata = 0;
                    continue;
                }

                // 오류 없으면 LCD에 이동된 뱀 그리기
                Snake_generator();
            }
        }
    }
}
