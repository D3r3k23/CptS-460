/******************* uart.c file ****************/ 
#define UDR 0x00
#define UFR 0x18

typedef struct uart{
  char *base;           // base address
  int  n;
}UART;

UART uart[4];  // 4 UART structs

// For versatile board: uarts at 0x101F1000-3000; 0x10009000
int uart_init()
{
  int i;
  UART *up;
  for (i=0; i<4; i++){
    up = &uart[i];
    up->base = (char *)(0x101f1000 + i*0x1000);
    up->n = i;
  }
  uart[3].base = (char *)(0x10009000);
}

int uputc(UART *up, char c)
{
  while ( *(up->base + UFR) & 0x20 );
  *(up->base + UDR) = (int)c;
}

int ugetc(UART *up)
{
  while ( *(up->base + UFR) & 0x10 );
  return (char)(*(up->base + UDR));
}

int ugets(UART *up, char *s)
{
  while ((*s = (char)ugetc(up)) != '\r'){
    uputc(up, *s);
    s++;
  }
 *s = 0;
}

int uputs(UART *up, char *s)
{
  while(*s){
    uputc(up, *s++);
    if (*s=='\n')
      uputc(up,'\r');
  }
}
