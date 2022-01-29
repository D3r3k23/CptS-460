#define UDR 0x00
#define UFR 0x18

typedef struct uart{  
  char *base;           // base address
  int  n;
}UART;

UART uart[4];          // 4 UART structs

// For versatile_epb : uarts are at 0x101F1000, 2000, 3000; 10009000 

int uart_init()
{
  int i; UART *up;
  for (i=0; i<4; i++){
    up = &uart[i];
    up->base = (char *)(0x101f1000 + i*0x1000);
    up->n = i;
  }
  uart[3].base = (char *)(0x10009000);
}


/******************************
Flag register (offset 0x18): status of UART port
   7    6    5    4    3    2   1    0
| TXFE RXFF TXFF RXFE BUSY  -   -    -|

where TXFE=Tx Empty, RXFF=Rx Full, TXFF=Tx Full, RXFE=Rx Empty
*****************************/

int ugetc(UART *up)
{
  while ( *(up->base + UFR) & 0x10 );  // while UFR.bi4=1 (RxEmpty: no data);
  return (char)(*(up->base + UDR));    // return *UDR
}

int uputc(UART *up, char c)
{
  while ( *(up->base + UFR) & 0x20 );  // while UFR.bit5=1 (TxFull: not empty);
  *(up->base + UDR) = (int)c;          // write c to UDR
}

int ugets(UART *up, char *s)
{
  while ((*s = (char)ugetc(up)) != '\r'){
    uputc(up, *s);
    s++;
  }
 *s = 0;
}

int uprints(UART *up, char *s)
{
  while(*s)
    uputc(up, *s++);
}
