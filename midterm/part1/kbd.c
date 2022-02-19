#include "keymap2"

#define LSHIFT 0x12
#define RSHIFT 0x59
#define ENTER  0x5A
#define LCTRL  0x14
#define RCTRL  0xE014

#define KCNTL 0x00
#define KSTAT 0x04
#define KDATA 0x08
#define KCLK  0x0C
#define KISTA 0x10

typedef volatile struct kbd{
  char *base;
  char buf[128];
  int head, tail;
  int data, room;
}KBD;

volatile KBD kbd;
int release = 0;

volatile int keyset;
int kputc(char);
extern int color;

int kbd_init()
{
  KBD *kp = &kbd;
  kp->base = (char *)0x10006000;
  *(kp->base + KCNTL) = 0x10; // bit4=Enable bit0=INT on
  *(kp->base + KCLK)  = 8;
  kp->head = kp->tail = 0;

  kp->data = 0;
  kp->room = 128;
   
  release = 0;
}

void kbd_handler()
{
  u8 scode, c;
  KBD *kp = &kbd;
  //  color = YELLOW;
  scode = *(kp->base + KDATA);

  if (scode == 0xF0){ // key release 
    release = 1;
    return;
  }
  if (release){      // key release code
    release = 0;
    return;
  }

  c = ltab[scode];
  if (c=='\r')
    kputc('\n');
  kputc(c);
  
  kp->buf[kp->head++] = c;
  kp->head %= 128;
  kp->data++; kp->room--;
}

int kgetc()
{
  char c;
  KBD *kp = &kbd;
  
  while(kp->data == 0);   // BUSY wait for data
  
  lock();
   c = kp->buf[kp->tail++];
   kp->tail %= 128;
   kp->data--; kp->room++;
  unlock();
  
  return c;
}

int kgets(char s[ ])
{
  char c;
  KBD *kp = &kbd;
  
  while( (c = kgetc()) != '\r'){
    if (c=='\b'){
      s--;
      continue;
    }
    *s++ = c;
  }
  *s = 0;
  return strlen(s);
}
