// vid.c file: implement fbuf for the ARM PL110 LCD display
/**************** Reference: ARM PL110 and DUI02241 ********************
Color LCD base address: 0x10120000 - 0x1012FFFF
00    timing0
04    timing1
08    timing2
0C    timing3
10    upperPanelframeBaseAddressRegister // use only ONE panel
14    lowerPanelFrameBaseAddressRegister // some display allows 2 panels
18    interruptMaskClrregister
1C    controlRegister
20    interruptStatusReg
etc
************************************************************************/
#include "font0"
char *tab = "0123456789ABCDEF";
int color;
u8 cursor;
int volatile *fb;
unsigned char *font;
int row, col;

int fbuf_init()
{
  int x; int i;
  //********* for VGA 640x480 ************************
  *(volatile u32 *)(0x1000001c) = 0x2C77;        // LCDCLK SYS_OSCCLK
  *(volatile u32 *)(0x10120000) = 0x3F1F3F9C;    // time0
  *(volatile u32 *)(0x10120004) = 0x090B61DF;    // time1
  *(volatile u32 *)(0x10120008) = 0x067F1800;    // time2
  *(volatile u32 *)(0x10120010) = (2*1024*1024); // panelBaseAddress at 2MB
  *(volatile u32 *)(0x10120018) = 0x80B; //82B;  // control register
  *(volatile u32 *)(0x1012001c) = 0x0;           // IntMaskRegister

  fb = (int *)(2*1024*1024);  // at 2MB area
  font = fonts0;              // use fonts0 for char bit patterns 

  // for 640x480 VGA mode display
  for (x=0; x<640*480; x++)
    fb[x] = 0x00000000;    // clear screen; all pixels are BLACK
  cursor = 127;            // cursor bit map in font0 at 127
}

int clrpix(int x, int y)
{
  int pix = y*640 + x;
  fb[pix] = 0x00000000;
}

int setpix(int x, int y)
{
  int pix = y*640 + x;
  if (color==RED)
     fb[pix] = 0x000000FF;
  if (color==BLUE)
     fb[pix] = 0x00FF0000;
  if (color==GREEN)
     fb[pix] = 0x0000FF00;
  if (color==CYAN)
     fb[pix] = 0x00FFFF00;
  if (color==YELLOW)
     fb[pix] = 0x0000FFFF;
}

int dchar(unsigned char c, int x, int y)
{
  int r, bit;
  u8 *caddress, byte;
  caddress = font + c*16;

  for (r=0; r<16; r++){
    byte = *(caddress + r);
    for (bit=0; bit<8; bit++){
      clrpix(x+bit, y+r);  // clear pixel to BALCK
      if (byte & (1<<bit))
	  setpix(x+bit, y+r);
    }
  }
}

int scroll()
{
  int i;
  for (i=0; i<640*480-640*16; i++){
    fb[i] = fb[i+ 640*16];
  } 
}
  
int kpchar(char c, int ro, int co)
{
   int x, y;
   x = co*8;
   y = ro*16;
   //printf("c=%x [%d%d] (%d%d)\n", c, ro,co,x,y);
   dchar(c, x, y);
   
}

int erasechar()
{ 
  // erase char at (row,col)
  int r, bit, x, y;
  x = col*8;
  y = row*16;

  for (r=0; r<16; r++){
     for (bit=0; bit<8; bit++){
        clrpix(x+bit, y+r);
    }
  }
} 

int clrcursor()
{
  erasechar();
}

int putcursor()
{
  kpchar(cursor, row, col);
}

int kputc(char c)
{
  clrcursor();
  if (c=='\r'){
    col=0;

    putcursor();
    return;
  }
  if (c=='\n'){
    row++;
    if (row>=25){
      row = 24;
      scroll();
    }

    putcursor();
    return;
  }
  if (c=='\b'){
    if (col>0){
      clrcursor();
      col--;
      putcursor();
    }
    return;
  }

  // c is ordinary char
  kpchar(c, row, col);
  col++;
  if (col>=80){
    col = 0;
    row++;
    if (row >= 25){
      row = 24;
      scroll();
    }
  }
  putcursor(); 
}


// write your own kprintf(char *fmt, ...) for formatted printing

int kputs(char *s)
{
  while(*s){
    kputc(*s);
    if (*s == '\n')
      kputc('\r');
    s++;
  }
}

int krpx(int x)
{
  char c;
  if (x){
     c = tab[x % 16];
     krpx(x / 16);
  }
  
  if (c >= '0' && c <= '9' || c >= 'A' && c <= 'F')
     kputc(c);
}

int kprintx(int x)
{
  kputc('0'); kputc('x');
  if (x==0)
    kputc('0');
  else
    krpx(x);
  kputs("\n");
}

