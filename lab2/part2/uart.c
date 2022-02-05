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
