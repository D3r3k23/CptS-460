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
/**************************
 need to catch Control-C, Contorl-D keys
 so need to recognize LCTRL key pressed and then C or D key
******************************/

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
extern PROC *running;

typedef volatile struct kbd{
  char *base;
  char buf[128];
  int head, tail, data, room;
}KBD;

int kputc(char);

volatile KBD kbd;
int release;
volatile int kline;

int kbd_init()
{
  KBD *kp = &kbd;
  kp->base = (char *)0x10006000;
  *(kp->base + KCNTL) = 0x10; // bit4=Enable bit0=INT on
  *(kp->base + KCLK)  = 8;
  kp->head = kp->tail = 0;
  kp->data = 0; kp->room = 128;
  release = 0;
}

void kbd_handler()
{
  u8 scode, c;
  KBD *kp = &kbd;

  scode = *(kp->base + KDATA);

  if (scode == 0xF0){ // begin key release 
    release = 1;
    return;
  }
  
  if (release){      // end key release
    release = 0;
    return;
  }

  c = ltab[scode];

  if (c != '\r')  
     kputc(c);

  kp->buf[kp->head++] = c;
  kp->head %= 128;
  kp->data++; kp->room--;

  if (c=='\r')
    kline++;
}

int kgetc()
{
  char c;
  KBD *kp = &kbd;

  unlock();
  while(kp->data == 0);

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


int kgetline(char s[ ])
{
  char c;
  KBD *kp = &kbd;
  
  if (kline==0){
     while(kline==0); // wait until kline > 0
  }
  while(1){
      c = kp->buf[kp->tail++];
      *s++ = c;
      kp->tail %= 128;
      kp->data--; kp->room++;
      if (c=='\r')
	break;
  }
  *(s-1) = 0;
  kline--;
}

