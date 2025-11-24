#include "my128.h"

void delay_ms(unsigned int m) {
    unsigned int i, j;

    for (i = 0; i < m; i++)
        for (j = 0; j < 2130; j++)
            ;
}

void delay_us(unsigned int u) {
    unsigned int i, j;

    for (i = 0; i < u; i++)
        for (j = 0; j < 2; j++)
            ;
}
