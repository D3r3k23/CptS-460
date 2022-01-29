/**************** t.c file **************/
#include "uart.c"

int v[10] = {1,2,3,4,5,6,7,8,9,10};
int sum;

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

int ufprintf(UART* up, char* fmt, ...)
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
                case 'c': uputc  (up,         (char)*arg_ptr); break;
                case 's': uprints(up,        (char*)*arg_ptr); break;
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

int main()
{
    int i;
    char line[128];
    UART *up;

    uart_init();
    up = &uart[0];

    while (1) {
        ufprintf(up, "%s", "enter a line : ");
        ugets(up, line);
        ufprintf(up, "%c", '\n');
        if (line[0] == 0) {
            break;
        }
        ufprintf(up, "line=%s\n", line);
    }

    ufprintf(up, "%s", "Compute sum of array\n");
    sum = 0;
    for (i = 0; i < 10; i++) {
        sum += v[i];
    }
    ufprintf(up, "sum=%d\n", sum);

    ufprintf(up, "%s", "END OF UART DEMO\n");
}
