#include "cfont"

#define BLUE  0
#define GREEN 1
#define RED   2

extern int color;
u8 cursor;
int volatile *fb;

int row, col;
unsigned char *font;  // cfont file : char cfonts[] = ASCI bitmaps 
int WIDTH = 640;

int fbuf_init()
{
    int i;
    fb = (int *)0x200000;
    font = cfonts;
    /********* for 640x480 ************************/
    *(volatile unsigned int *)(0x1000001c) = 0x2C77;
    *(volatile unsigned int *)(0x10120000) = 0x3F1F3F9C;
    *(volatile unsigned int *)(0x10120004) = 0x090B61DF;
    *(volatile unsigned int *)(0x10120008) = 0x067F1800;
    *(volatile unsigned int *)(0x10120010) = 0x200000;
    *(volatile unsigned int *)(0x10120018) = 0x82B;
    /********** for 800X600 **********************
    *(volatile unsigned int *)(0x1000001c) = 0x2CAC; // 800x600
    *(volatile unsigned int *)(0x10120000) = 0x1313A4C4;
    *(volatile unsigned int *)(0x10120004) = 0x0505F6F7;
    *(volatile unsigned int *)(0x10120008) = 0x071F1800;
    *(volatile unsigned int *)(0x10120010) = 0x200000;
    *(volatile unsigned int *)(0x10120018) = 0x82B;
    **********/
    cursor = 127; // cursor bit map in font0 at 127
    for (i=0; i<480*640; i++){
	fb[i] = 0;   // black screen
    } 
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
}

int dchar(unsigned char c, int x, int y)
{
  int r, bit;
  unsigned char *caddress, byte;

  caddress = font + c*16;       // 16-bytes per ASCII in font[ ]
  
  for (r=0; r<16; r++){         // each ASCII char is a 16row x 8col bitmap
    byte = *(caddress + r);     // get each byte of a row

    for (bit=0; bit<8; bit++){  // for each bit in this byte 
      if (byte & (1<<bit))      // set pixel by 1 bits in this byte
	 setpix(x+bit, y+r);    // x inc, but same y+r
    }
  }
}

int undchar(unsigned char c, int x, int y)
{
  int r, bit;
  unsigned char *caddress, byte;

  caddress = font + c*16;

  for (r=0; r<16; r++){
    byte = *(caddress + row);

    for (bit=0; bit<8; bit++){
        if (byte & (1<<bit))    // OR simply clr all pixels in this row
	   clrpix(x+bit, y+r);
    }
  }
} 

int scroll()
{
  int i;
  for (i=64*640; i<640*480; i++){
    fb[i] = fb[i + 640*16];
  } 
}

int kpchar(char c, int ro, int co) // show c at (row=ro, col=co)
{
   int x, y;
   x = co*8;           // pixel location (x,y) in 480x640 matrix
   y = ro*16;
   dchar(c, x, y); 
}

int unkpchar(char c, int ro, int co)
{
   int x, y;
   x = co*8;
   y = ro*16;
   undchar(c, x, y);
}

int clrcursor()
{
  unkpchar(127, row, col);     // clear cursor at (row,col)
}

int putcursor(unsigned char c) // show cursor at (row,col)
{
  kpchar(c, row, col);
}
 
int kputc(char c)              // display a char at (row, col)     
{                              // advance cursor 
  clrcursor();
  if (c=='\r'){
    col=0;
    putcursor(cursor);
    return;
  }
  if (c=='\n'){
    row++;
    if (row>=25){
      row = 24;
      scroll();
    }
    putcursor(cursor);
    return;
  }
  if (c=='\b'){
    if (col>0){
      col--;
      putcursor(cursor);
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
  putcursor(cursor); 
}

int kputs(char *s)
{
  while(*s){
    kputc(*s);
    if (*s == '\n')
      kputc('\r');
    s++;
  }
}
