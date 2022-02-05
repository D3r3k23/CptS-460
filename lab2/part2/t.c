#include "uart.c"
#include "vid.c"

int color;

extern char _binary_wsu_bmp_start;

int color;
UART *up;

int main()
{
    char *p;

    uart_init();
    up = &uart[0];

    fbuf_init();

    p = &_binary_wsu_bmp_start;
    show_bmp(p, 0, 0);

    while (1); // loop here
}
