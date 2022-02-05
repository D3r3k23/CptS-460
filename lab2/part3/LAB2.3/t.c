/********************* t.c file ******************/
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

#include "uart.c"
#include "vid.c"

int color;
int row, col;

int main()
{
   char line[64];
   UART *up;
   
   uart_init();
   up = &uart[0];

   fbuf_init();
   row = 0; col = 0;

   while(1){
     color = GREEN;
     kputs("enter a line from UART port0\n");
     uputs(up, "enter line from this UART : ");
     ugets(up, line);
     uputs(up, "  line="); uputs(up, line); uputs(up, "\n");
     color = RED;
     kputs("line="); kputs(line); kputs("\n");
   }
   while(1);   // loop here  
}
