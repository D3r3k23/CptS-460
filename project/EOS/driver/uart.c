/********************************************************************
Copyright 2010-2017 K.C. Wang, <kwang@eecs.wsu.edu>
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

#include "../type.h"

// uart.c file
/*
UART0 base address: 0x101f1000;
UART1 base address: 0x101f2000;
UART2 base address: 0x101f3000;
UART3 base address: 0x10009000;

#define UART0_BASE_ADDR 0x101f1000
#define UART0_DR   (*((volatile u32 *)(UART0_BASE_ADDR + 0x000)))
#define UART0_FR   (*((volatile u32 *)(UART0_BASE_ADDR + 0x018)))
#define UART0_IMSC (*((volatile u32 *)(UART0_BASE_ADDR + 0x038)))

#define UART1_BASE_ADDR 0x101f2000
#define UART1_DR   (*((volatile u32 *)(UART1_BASE_ADDR + 0x000)))
#define UART1_FR   (*((volatile u32 *)(UART1_BASE_ADDR + 0x018)))
#define UART1_IMSC (*((volatile u32 *)(UART1_BASE_ADDR + 0x038)))

// UART's flag register at 0x18
//  7    6    5    4    3    2   1   0
// TXFE RXFF TXFF RXFE BUSY
// TX FULL : 0x20
// TX empty: 0x80
// RX FULL : 0x40
// RX empty: 0x10
// BUSY=1 :  0x08
*/
int kprintf(char *fmt, ...);
/******
#define DR      0
#define SR      4
#define FR     24  
#define BUFLEN 64

typedef volatile struct uart{
   char *base;               // base = DR at 0

   // input buffer 
   char inbuf[BUFLEN];
   int inhead, intail;
   struct semaphore inchar;

   // output buffer 
   char outbuf[BUFLEN];
   int outhead, outtail;
   struct semaphore outspace;
   char kline[LSIZE];
   int tx_on;
   
   // echo buffer 
   char ebuf[BUFLEN];
   int ehead, etail, e_count;


}UART;
*********/

UART uart[4];  // 4 UART pointers to their base addresses
UART *up;

int uart_init()
{
  int i;
  UART *up;
  kprintf("uart[0-4] init()\n");
  for (i=0; i<4; i++){         // uart0 to uart2 are adjacent
    up = &uart[i];
    up->base = (char *)(0x101F1000 + i*0x1000); // up->DR 
    if (i==3)
       up->base = (char *)(0x10009000); // uart3 at 0x10009000

    up->inhead = up->intail = 0;
    up->inchar.value = 0; up->inchar.queue = 0;
 
    up->outhead = up->outtail = up->tx_on = 0;  
    up->outspace.value = BUFLEN; up->outspace.queue = 0;

    up->ehead =  up->etail = up->e_count = 0;
  }
}

extern PROC proc[];

void uart0_handler() {
  char c, d;
  UART *up;
  volatile char *t, *tt;
  int i, count;

  color = GREEN;

  up = &uart[0];
  while( *(up->base+FR) & 0x40 ==0 ); 

  c = *(up->base + DR);
  //printf("uart0 interrupt c=%x %c\n", c, c);

  if (c==0x3){
    printf("Control_C : send signal#2 to procs ");
    for (i=1; i<NPROC; i++){  // give signal#2 to ALL on this terminal
      if (proc[i].status != FREE && strcmp(proc[i].res->tty, "/dev/ttyS0")==0){
	proc[i].res->signal |= (1 << 2); // sh IGNore, so only children die
      }   
    }
    printf("\n");
    c = '\r'; // force a line, let proc handle #2 signal when exit Kmode
  }
  //printf("head=%d tail=%d ", up->inhead, up->intail);  
  up->inbuf[up->inhead++] = c;
  up->inhead %= BUFLEN;
  V(&up->inchar);       // inchar.value++; including '\r'
  color=RED;
}


void uart1_handler() {
  char c;
  UART *up;
  int i;
  color=WHITE;
  
  up = &uart[1];
  while( *(up->base+FR) & 0x40 == 0 ); 

  c = *(up->base + DR);
  //printf("uart1 interrupt c=%x %c\n", c, c);

  if (c==0x3){
    printf("Control_C : send signal#2 to procs ");
    for (i=1; i<NPROC; i++){  // give signal#2 to ALL on this terminal
      if (proc[i].status != FREE && strcmp(proc[i].res->tty, "/dev/ttyS1")==0){
	proc[i].res->signal |= (1 << 2); // sh IGNore, so only children die
      }   
    }
    printf("\n");
    c = '\r'; // force a line, let proc handle #2 signal when exit Kmode
  }

  up->inbuf[up->inhead++] = c;
  up->inhead %= BUFLEN;
  V(&up->inchar);       // inchar.value++; including '\r'

  color=RED;
}

// TO DO: UART outputs should be intertupt-driven also 

int sputc(UART *up, char c)
{
  int i = *(up->base + FR);
  while(*(up->base + FR) & 0x20);
  // printf("sputc: c=%c ", c);
  *(up->base + DR) = (char)c;
}

int sgetc(UART *up)
{
  char c;

  P(&up->inchar);

  c = up->inbuf[up->intail++];
  up->intail %= BUFLEN;
  //printf("%d past P c=%x ", running->pid, c);
  return c;
}

int sgets(UART *up, char *s)
{
  while ((*s = (char)sgetc(up)) != '\r'){
    sputc(up, *s);
    s++;
  }
 *s = 0;
}

int sputs(char *s)
{
  while(*s){
    sputc(up, *s++);
    if (*s=='\n')
      sputc(up,'\r');
  }
   
}

int sprints(UART *up, char *s)
{
  while(*s)
    sputc(up, *s++);}

int srpx(UART *up, int x)
{
  char c;
  if (x){
     c = tab[x % 16];
     srpx(up, x / 16);
  }
  sputc(up, c);
}

int sprintx(UART *up, int x)
{
  sprints(up, "0x");
  if (x==0)
    sputc(up, '0');
  else
    srpx(up, x);
  sputc(up, ' ');
}

int srpu(UART *up, int x)
{
  char c;
  if (x){
     c = tab[x % 10];
     srpu(up, x / 10);
  }
  sputc(up, c);
}

int sprintu(UART *up, int x)
{
  if (x==0)
    sputc(up, '0');
  else
    srpu(up, x);
  sputc(up, ' ');
}

int sprinti(UART *up, int x)
{
  if (x<0){
    sputc(up, '-');
    x = -x;
  }
  sprintu(up, x);
}

int sfprintf(UART *up, char *fmt,...)
{
  int *ip;
  char *cp;
  cp = fmt;
  ip = (int *)&fmt + 1;

  while(*cp){
    if (*cp != '%'){
      sputc(up, *cp);
      if (*cp=='\n')
	sputc(up, '\r');
      cp++;
      continue;
    }
    cp++;
    switch(*cp){
    case 'c': sputc(up, (char)*ip);      break;
    case 's': sprints(up, (char *)*ip);  break;
    case 'd': sprinti(up, *ip);           break;
    case 'u': sprintu(up, *ip);           break;
    case 'x': sprintx(up, *ip);  break;
    }
    cp++; ip++;
  }
}

int ksprintf(char *fmt, ...)
{
  int *ip;
  char *cp;
  cp = fmt;
  ip = (int *)&fmt + 1;

  UART *up = &uart[0];

  while(*cp){
    if (*cp != '%'){
      sputc(up, *cp);
      if (*cp=='\n')
	sputc(up, '\r');
      cp++;
      continue;
    }
    cp++;
    switch(*cp){
    case 'c': sputc(up, (char)*ip);      break;
    case 's': sprints(up, (char *)*ip);   break;
    case 'd': sprinti(up, *ip);           break;
    case 'u': sprintu(up, *ip);           break;
    case 'x': sprintx(up, *ip);  break;
    }
    cp++; ip++;
  }
}

int sgetline(char *s) // from uart[0] only
{
  char c;
  UART *up = &uart[0];

  while ((c=sgetc(up)) != '\r'){
    *s++ = c;
  }
 *s = 0;
}

int usgets(UART *up, char *buf)
{
  // get a line from uart to buf[] in Umode
  char c;
  while((c=sgetc(up)) != '\r'){
    *buf++ = c;
  }
  *buf = 0;
  return kstrlen(buf);
}
