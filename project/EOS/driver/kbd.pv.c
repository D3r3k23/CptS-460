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

/*************************  6-3, 06 *************************************
5-2-06 : Re-wrote the kbd.c completely. The only thing left from MINIX's kbd.c
is part of the scan code to ASCII translation table. Instead of mapping the 
special keys to weired octal values, they are handled directly.
**************************************************************************/
#include "../type.h"

#define N_SCAN           64	/* Number of scan codes */
#define F1		 0x3B
#define F2		 0x3C

#define CAPSLOCK         0x3A
#define LSHIFT           0x2A
#define RSHIFT           0x36
#define CONTROL          0x1D
#define ALT              0x38
#define DEL              0x53

#define KBN       128

#include "keymap"

volatile KBD *kbd;
volatile int *kdata = (int *)0x10006008;

volatile char kbbuf[KBN];
volatile int kbhead, kbtail;
volatile int kbline;

struct semaphore kbData;
//struct semaphore kbline;

extern struct semaphore fdsem; // defined in fd.c

int color;
int alt;		/* alt key state */
int capslock;		/* caps lock key state */
int esc;		/* escape scan code detected? */
int control;		/* control key state */
int shift;		/* left and right shift key state */
int escKey;             // keys that come as escape sequence E0 xx
int kputc(char);

int kbd_init()
{
  kprintf("kbd_init()\n");
  kbd = (KBD *)(0x10006000);
  kbd->control = 0x14; // 0001 0100
  kbd->clock = 8;
  //cursor = 128;      // cursor icon set in vid.c fbuf_init()

  shift=alt=capslock=esc=control=0;
  kbhead = kbtail = 0;
  kbData.value = 0;   kbData.queue = 0;
  kbline = 0;
}

/************************************************************************
 kbhandler() is the kbd interrupt handler. The kbd generates 2 interrupts
 for each key typed; one when the key is pressed and another one when the
 key is released. Each key generates a scan code. The scan code of a key
 release is 0x80 + the scan code of key pressed. When the kbd interrupts,
 the scan code is in the data port (0x60) of the KBD interface. First, 
 read the scan code from the data port. Then ack the key input by strobing 
 its PORT_B at 0x61.
 Some special keys generate ESC key sequences,e.g. arrow keys
                   Then process the scan code:
1. Normal key releases are ignored except for the spcecial keys of 
   0xE0, CAPLOCK, CONTROL, ALT, SHIFTs, which are used to set or clear
   the status variables  esc, control, alt, shift  
2. For normal keys: translate into ASCII, depending on shifted or not.
3. ASCII keys are entered into a circular input buffer. 
4. Sync between upper routines and lower routines is by P/V on semaphores
   kbData, kbline
5. The input buffer contains RAW keys (may have \b). kgets()/ugets() cooks
   the keys to weed out specail keys such as \b. So far only \b is handled.
   Arrow keys are used only by sh for history. Each arrow key is immediately
   made into a line to allow sh to get a line of inputs.   
**************************************************************************/

int doF1()
{
  //kps(); 
  printf("running=%d\n", running->pid);
}
int doF2()
{
    printQ(readyQueue);
}


void kbd_handler()
{
  unsigned char code, c;
  volatile char *t, *tt;
  int i;

  //color=RED;
  while(kbd->status & 0x20);    // while TxBusy OR RXbusy
  while(kbd->status & 0x08);    // while TxBusy OR RXbusy
  while(!(kbd->status & 0x10)); // while RX not full

  code = KBD_DR;

  //  if ((code & 0x80)==0)
  //   printf("%x ", code);

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

     kbbuf[kbhead++] = escKey;
     kbhead %= KBN;
     V(&kbData); 

     kbbuf[kbhead++] = '\n';
     kbhead %= KBN;

     V(&kbData);
     V(&kbline);
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
        //printf("%d ", proc[i].pid);
        //kkill(2, proc[i].pid);  // kill unblocks process
      }   
    }
    printf("\n");
    c = '\n'; // force a line, let proc handle #2 signal when exit Kmode
  }

  if (control && (c=='d'|| c=='D')){  // Control-D, these are 2 keys
    c = 4;   // Control-D
  }

  /* Store the character in kbbuf[] for task to get */
  if (kbData.value == KBN)
     goto out;

  kbbuf[kbhead++] = c;
  kbhead %= KBN;

  kputc(c);
  if (c =='\r')
     kputc('\n');

  V(&kbData); 
  if (c=='\r')
    kbline++;
 out:          
  kbd->status = 0xFF;
  VIC_VADDR = 0xFF;
}

/********* KBD driver upper half rotuines ***********/ 

int kgetc()
{
  int c;

  P(&kbData);
  c = kbbuf[kbtail++] & 0x7F;
  kbtail %= KBN;
  return c;
}


int kgetline(char s[ ])
{
  char c;
  while((c=kgetc()) != '\r'){
    *s = c;
    //kputc(c);
    s++;
  }
  // last char was \r
  *s = 0;
  //  kputc(c); kputc('\n');
}


/*
int kgetline(char s[ ])
{
  char c;
  if (kbline==0){
     kprintf("enter a line from KBD: ");
     while(kbline==0); // wait until kline > 0
  }
  // fetch a line from kbuf[ ] 

  while(1){
      c = kbbuf[kbtail++];
      *s++ = c;
      kbtail %= 128;
      kbData.value--;
      if (c=='\r')
	break;
  }
  *(s-1) = 0;
  kbline--;
}
*/
