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
#include "../type.h"
#include "keymap"
#include "keymap2"
/*
#define LSHIFT 0x12
#define RSHIFT 0x59
#define ENTER  0x5A
#define LCTRL  0x14
#define RCTRL  0xE014
*/

#define KCNTL 0x00
#define KSTAT 0x04
#define KDATA 0x08
#define KCLK  0x0C
#define KISTA 0x10

extern PROC *running;
/*
typedef volatile struct kbd{
  char *base;
  char buf[128];
  int head, tail, data, room;
}KBD;
*/
int kputc(char);

volatile KBD kbd;
int shifted = 0;
int release = 0;
int control = 0;
volatile int kline;
volatile int keyset;

//extern PROC proc[NPROC];
int kbd_init()
{
  char scode;
  keyset = 1; // default to scan code set-1
  
  KBD *kp = &kbd;
  kp->base = (char *)0x10006000;
  *(kp->base + KCNTL) = 0x10; // bit4=Enable bit0=INT on
  *(kp->base + KCLK)  = 8;
  kp->head = kp->tail = 0;
  kp->data = 0; kp->room = 128;
  shifted = 0;
  release = 0;
  control = 0;

  printf("Detect KBD scan code: press the ENTER key : ");
  while( (*(kp->base + KSTAT) & 0x10) == 0);
  scode = *(kp->base + KDATA);
  printf("scode=%x ", scode);
  if (scode==0x5A)
    keyset=2;
  printf("keyset=%d\n", keyset);
}

// kbd_handler1() for scan code set 1

#define F1		 0x3B
#define F2		 0x3C
#define F3		 0x3D
#define F4		 0x3E

#define CAPSLOCK         0x3A
#define LSHIFT           0x2A
#define RSHIFT           0x36
#define CONTROL          0x1D
#define ALT              0x38
#define DEL              0x53

#define KBN       128

int color;
int alt;		/* alt key state */
int capslock;		/* caps lock key state */
int esc;		/* escape scan code detected? */
int control;		/* control key state */
int shift;		/* left and right shift key state */
int escKey;             // keys that come as escape sequence E0 xx

int doF1()
{
  printf("running=%d\n", running->pid);
}
int doF2()
{
    printQ(readyQueue);
}
int doF3()
{
  printSleepList(sleepList);
}
int doF4()
{
  kps();
}

void kbd_handler1()
{
  unsigned char code, c;
  int i;
  KBD *kp = &kbd;

  code = *(kp->base + KDATA);
  
  //  if ((code & 0x80)==0)
  //printf("%x ", code);

  // must catch 0xE0 for keys that generate escape (scan) code sequences:
  // Arrow keys: up=E048; down=E050; left=E04B; right=E04D
  // Rcontrol=E01D        Delete=E053/ RAlt=E038

  if (code == 0xE0) 
     esc++;              // inc esc bount by 1

  if (esc && esc < 2)    // only the first 0xE0, wait for next code
    goto out;

  if (esc == 2){ // two E0 means escape sequence key release
    if (code == 0xE0) // this is the 2nd E0 itself => real code will come next
       goto out;

     // with esc==2, this must be the actual scan code, so handle it 

     code &= 0x7F;         // leading bit off 

     if (code == 0x53){   // Delete key ONLY USE is control-alt-del
       //if (control && alt) // this is NOT the Del key, so very unlikely
	 // kreboot();        // to reboot by Cntl-Alt-Delete 
       goto out;
     }

     if (code == 0x38){  // Right Alt
       alt = 0;
       goto out;
     }

     if (code == 0x1D){   // Right Control release
       control = 0;
       goto out;
     }

     if (code == 0x48)   // up arrow
       escKey = 0x0B;
     if (code == 0x50)   // down arrow
       escKey = 0x0C;
     if (code == 0x4B)   // left arrow
       escKey = 0x0D;
     if (code == 0x4D)   // right arrow
       escKey = 0x0E;

     kp->buf[kp->head++] = escKey;
     kp->head %= KBN;
     //V(&kbData); 
     kp->data++;
      
     kp->buf[kp->head++] = '\n';
     kp->head %= KBN;
     kp->data++; kp->room--;

     //V(&kbData);
     kwakeup(&kp->data);

     esc = 0;
     goto out;
  }

  if (code & 0x80){ // key release: ONLY catch release of shift, control

    code &= 0x7F;  // mask out bit 7
    if (code == LSHIFT || code == RSHIFT)
      shift = 0;    // released the shift key
    if (code == CONTROL)
      control = 0;
    if (code == ALT)
      alt = 0;
    goto out;
  }

  // from here on, must be key press 
  if (code == LSHIFT || code == RSHIFT){
    shift = 1;
    goto out;
  }
  if (code == ALT){
    alt = 1;
    goto out;
  }
  if (code == CONTROL){
    control = 1;
    goto out;
  }

  if (code == 0x3A){
    capslock = 1 - capslock;    // capslock key acts like a toggle
    goto out;
  }

  if (control && alt && code == DEL){
    //kreboot();
      goto out;
  }
  /************ 5-06-****** add F keys for debuggin *************/
  if (code == F1){
     doF1(); goto out;
  }
  if (code == F2){
     doF2(); goto out;
  }
  if (code == F3){
     doF3(); goto out;
  }
  if (code == F4){
     doF4(); goto out;
  }

  c = (shift ? sh[code] : unsh[code]);

  if (capslock){
    if (c >= 'A' && c <= 'Z')
	c += 'a' - 'A';
    else if (c >= 'a' && c <= 'z')
	c -= 'a' - 'A';
  }

  if (control && c=='c'){  // Control-C keys on PC, these are 2 keys
    //printf("Control_C : send signal#2 to procs ");
    for (i=1; i<NPROC; i++){  // give signal#2 to ALL on this terminal
      if (proc[i].status != FREE && strcmp(proc[i].res->tty, "/dev/tty0")==0){
	proc[i].res->signal |= (1 << 2); // sh IGNore, so only children die
      }   
    }
    c = '\n'; // force a line, let proc handle #2 signal when exit Kmode
  }

  if (control && (c=='d'|| c=='D')){  // Control-D, these are 2 keys
    c = 4;   // Control-D
  }

  /* Store the character in buf[] for task to get */
  //if (kbData.value == KBN)
  if (kp->data == KBN)
     goto out;

  kp->buf[kp->head++] = c;
  kp->head %= KBN;

  kp->data++;
  kwakeup(&kp->data);
 
  if (c=='\r'){
    kline++;
    kwakeup(&kline);
  }
 out:          
  *(kp->base + KSTAT) = 0xFF;
  VIC_VADDR = 0xFF;

}

/*******************
void kbd_handler1()
{
  u8 scode, c;
  int i;
  KBD *kp = &kbd;
  color = YELLOW;
  scode = *(kp->base + KDATA);

  //printf("scan code = %x ", scode);
  
  if (scode & 0x80)
    return;
  c = unsh[scode];

  if (c >= 'a' && c <= 'z')
     printf("kbd interrupt: c=%x %c\n", c, c);

  //kputc(c);
  
  kp->buf[kp->head++] = c;
  kp->head %= 128;
  kp->data++; kp->room--;

  if (c=='\r')
    kline++;
}
*****************************/

// kbd_handelr2() for scan code set 2
#define SHIFT  0x12
#define LCTRL  0x14
#define RCTRL  0xE014

void kbd_handler2()
{
  u8 scode, c;
  int i;
  KBD *kp = &kbd;
  //color = YELLOW;
  scode = *(kp->base + KDATA);
  //  printf("scanCode = %x  ", scode);

  if (scode == 0xF0){ // key release 
    release = 1;
    return;
  }
  
  if (release && scode != 0x12){ // ordinay key release
    release = 0;
    return;
  }

  if (release && scode == 0x12){ // Left shift key release
    release = 0;
    shifted = 0;
    return;
  }

  if (!release && scode == 0x12){// left key press and HOLD
    release = 0;
    shifted = 1;
    return;
  }

  if (!release && scode == LCTRL){// left Control key press and HOLD
    release = 0;
    control = 1;
    return;
  }

  if (release && scode == LCTRL){ // Left Control key release
    release = 0;
    control = 0;
    return;
  }
    
  if (!shifted)
     c = ltab[scode];
  else
     c = utab[scode];
  
  /********* catch Control-C ****************/
  if (control && scode == 0x21){ // Control-C
    // send signal 2 to processes on KBD
    printf("Control-C: scode=%x\n", scode);
    for (i=1; i<NPROC; i++){  // give signal#2 to ALL on this terminal
      if (proc[i].status != FREE && strcmp(proc[i].res->tty, "/dev/tty0")==0){
	proc[i].res->signal |= (1 << 2); // sh IGNore, so only children die
      }   
    }
    printf("\n");
    c = '\r'; // force a line, let proc handle #2 signal when exit Kmode
   
    control = 0;
  }   

  if (control && scode == 0x23){ // Control-D
    c = 0x4;
    printf("Control-D: c = %x\n", c);
    control = 0;
  }   

  if (control && scode == 0x23){ // Control-D
    c = 0x04;
    printf("Control-D: c = %x\n", c);
  }   

  kp->buf[kp->head++] = c;
  kp->head %= 128;
  kp->data++; kp->room--;
  kwakeup(&kp->data);
  
  if (c=='\r'){
    kline++;
    kwakeup(&kline);
  }
}

void kbd_handler()
{
  if (keyset == 1)
    kbd_handler1();
  else
    kbd_handler2();
}


int kgetc()
{
  char c;
  KBD *kp = &kbd;
  //printf("%d in kgetc\n", running->pid); 
  lock();

  while(kp->data == 0){
    unlock();
    ksleep(&kp->data);
    lock();
  }

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
/*
int getc()  // 
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
*/
int mgetc()  // 
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

int kgetline(char s[ ])
{
  char c;
  KBD *kp = &kbd;
  
  while (kline==0){
    ksleep(&kline);
  }
  // fetch a line from kbuf[ ] 
  lock();
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
  unlock();
}

