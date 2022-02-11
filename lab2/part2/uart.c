/**************** uart.c file ****************/
#define UDR 0x00
#define UFR 0x18

#define RXFE 0x10
#define TXFF 0x20

typedef struct uart {
  char *base;
  int n;
} UART;

UART uart[4];

int uart_init()
{
    int i; UART *up;

    for (i=0; i<4; i++) {
        up = &uart[i];
        up->base = (char *)(0x101F1000 + i*0x1000);
        up->n = i;
    }
    uart[3].base = (char *)(0x10009000); // uart3 at 0x10009000
}

int ugetc(UART *up)
{
    while (*(up->base + UFR) & RXFE);
    return *(up->base + UDR);
}

int uputc(UART *up, char c)
{
    while (*(up->base + UFR) & TXFF);
    *(up->base + UDR) = c;
}

int ugets(UART *up, char *s)
{
    while ((*s = (char)ugetc(up)) != '\r') {
        uputc(up, *s);
        s++;
    }
    *s = 0;
}

int uputs(UART *up, char *s)
{
    while (*s) {
        uputc(up, *s);
        if (*s == '\n')
            uputc(up, '\r');
        s++;
    }
}

/** WRITE YOUR uprintf(UART *up, char *fmt, . . .) for formatted print **/

char* tab = "0123456789ABCDEF";

int urpu(UART* up, unsigned int x)
{
    char c;
    if (x) {
        c = tab[x % 10];
        urpu(up, x / 10);
        uputc(up, c);
    }
}

int xrpu(UART* up, unsigned int x)
{
    char c;
    if (x) {
        c = tab[x / 16];
        urpu(up, x / 16);
        uputc(up, c);
    }
}

int uprintu(UART* up, unsigned int x)
{
    if (x == 0) {
        uputc(up, '0');
    } else {
        urpu(up, x);
    }
}

int uprintx(UART* up, unsigned int x)
{
    if (x == 0) {
        uputc(up, '0');
    } else {
        xrpu(up, x);
    }
}

int uprinti(UART* up, int x)
{
    if (x < 0) {
        uputc(up, '-');
        x = -x;
    }
    uprintu(up, x);

}

int uprintf(UART* up, char* fmt, ...)
{
    int* arg_ptr;
    int i;
    char c;

    arg_ptr = (int*)&fmt + 1;
    i = 0;

    while (c = *fmt) {
        if (c == '%') {
            fmt++;
            c = *fmt;
            switch (c) {
                case 'c': uputc(  up,         (char)*arg_ptr); break;
                case 's': uputs(  up,        (char*)*arg_ptr); break;
                case 'd': uprinti(up,          (int)*arg_ptr); break;
                case 'u': uprintu(up, (unsigned int)*arg_ptr); break;
                case 'x': uprintx(up, (unsigned int)*arg_ptr); break;
                default:
                    uputc(up, '%');
                    continue;
            }
            arg_ptr++;
        }
        else {
            uputc(up, c);
        }
        fmt++;
    }
}
