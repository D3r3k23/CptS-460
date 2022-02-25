#include "font0"
char *tab = "0123456789ABCDEF";
u8 cursor;
int volatile *fb;
u8 *font;
int row, col;
int color;

int fbuf_init()
{
  int x; int i;

  //********* for VGA 640x480 ************************
  *(volatile u32 *)(0x1000001c) = 0x2C77;        // LCDCLK SYS_OSCCLK
  *(volatile u32 *)(0x10120000) = 0x3F1F3F9C;    // time0
  *(volatile u32 *)(0x10120004) = 0x090B61DF;    // time1
  *(volatile u32 *)(0x10120008) = 0x067F1800;    // time2
  *(volatile u32 *)(0x10120010) = (1*1024*1024); // panelBaseAddress
  *(volatile u32 *)(0x10120018) = 0x80B; //82B;  // control register
  *(volatile u32 *)(0x1012001c) = 0x0;           // IntMaskRegister

  fb = (int *)(1*1024*1024);  // at 1MB area; enough for 800x600 SVGA
  font = fonts0;              // use fonts0 for char bit patterns 

  // for 640x480 VGA mode display
  for (x=0; x<640*480; x++)
    fb[x] = 0x00000000;    // clean screen; all pixels are BLACK
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
 if (color==GREEN)
     fb[pix] = 0x0000FF00;
  if (color==CYAN)
     fb[pix] = 0x00FFFF00;
  if (color==YELLOW)
     fb[pix] = 0x0000FFFF;
  if (color==PURPLE)
     fb[pix] = 0x00FF00FF;
  if (color==WHITE)
     fb[pix] = 0x00FFFFFF;

}

int dchar(unsigned char c, int x, int y)
{
  int r, bit;
  u8 *caddress, byte;
  caddress = font + c*16;
  //  printf("c=%x %c caddr=%x\n", c, c, caddress);
  for (r=0; r<16; r++){
    byte = *(caddress + r);
    for (bit=0; bit<8; bit++){
      clrpix(x+bit, y+r);  // clear pixel to BALCK
      if (byte & (1<<bit))
	  setpix(x+bit, y+r);
    }
  }
}
int undchar(unsigned char c, int x, int y)
{
  int row, bit;
  unsigned char *caddress, byte;

  caddress = font + c*16;
  //  printf("c=%x %c caddr=%x\n", c, c, caddress);

  for (row=0; row<16; row++){
    byte = *(caddress + row);

    for (bit=0; bit<8; bit++){
      if (byte & (1<<bit))
	  clrpix(x+bit, y+row);
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

int unkpchar(char c, int ro, int co)
{
   int x, y;
   x = co*8;
   y = ro*16;
   //printf("c=%x [%d%d] (%d%d)\n", c, ro,co,x,y);
   undchar(c, x, y);
}

int erasechar()
{ 
  // erase char at (row,col)
  int r, bit, x, y;
  x = col*8;
  y = row*16;
 
  //printf("ERASE: row=%d col=%d x=%d y=%d\n",row,col,x,y);
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
    //printf("row=%d col=%d\n", row, col);
    putcursor();
    return;
  }
  if (c=='\n'){
    row++;
    if (row>=25){
      row = 24;
      scroll();
    }
    //printf("row=%d col=%d\n", row, col);
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
  //printf("row=%d col=%d\n", row, col);
}

int kprints(char *s)
{
  while(*s){
    kputc(*s);
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
  if (c>='0' && c<='9' || c>='A' && c<='F')
     kputc(c);
}

int kprintx(int x)
{
  kputc('0'); kputc('x');
  if (x==0)
    kputc('0');
  else
    krpx(x);
  kputc(' ');
}

int krpu(int x)
{
  char c;
  if (x){
     c = tab[x % 10];
     krpu(x / 10);
  }
  if (c>='0' && c<='9')
     kputc(c);
}

int kprintu(int x)
{
  if (x==0)
    kputc('0');
  else
    krpu(x);
  kputc(' ');
}

int kprinti(int x)
{
  if (x<0){
    kputc('-');
    x = -x;
  }
  kprintu(x);
}

int kprintf(char *fmt,...)
{
  int *ip;
  char *cp;
  cp = fmt;
  ip = (int *)&fmt + 1;

  while(*cp){
    if (*cp != '%'){
      kputc(*cp);
      if (*cp=='\n')
	kputc('\r');
      cp++;
      continue;
    }
    cp++;
    switch(*cp){
    case 'c': kputc((char)*ip);      break;
    case 's': kprints((char *)*ip);  break;
    case 'd': kprinti(*ip);          break;
    case 'u': kprintu(*ip);          break;
    case 'x': kprintx(*ip);          break;
    }
    cp++; ip++;
  }
}
