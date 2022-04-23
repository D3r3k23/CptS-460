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

// timer.c file

#define CTL_ENABLE          ( 0x00000080 )
#define CTL_MODE            ( 0x00000040 )
#define CTL_INTR            ( 0x00000020 )
#define CTL_PRESCALE_1      ( 0x00000008 )
#define CTL_PRESCALE_2      ( 0x00000004 )
#define CTL_CTRLEN          ( 0x00000002 )
#define CTL_ONESHOT         ( 0x00000001 )

extern volatile int swflag;

typedef volatile struct timer{
  u32 LOAD;     // Load Register, TimerXLoad                             0x00
  u32 VALUE;    // Current Value Register, TimerXValue, read only        0x04
  u32 CONTROL;  // Control Register, TimerXControl                       0x08
  u32 INTCLR;   // Interrupt Clear Register, TimerXIntClr, write only    0x0C
  u32 RIS;      // Raw Interrupt Status Register, TimerXRIS, read only   0x10
  u32 MIS;      // Masked Interrupt Status Register,TimerXMIS, read only 0x14
  u32 BGLOAD;   // Background Load Register, TimerXBGLoad                0x18
  u32 *base;
}TIMER;

volatile TIMER *tp[4];  // 4 timers; 2 timers per unit; at 0x00 and 0x20
u8 btime[8];            // base time Intel x86: saved BIOS time in BCD form

// timer0 base=0x101E2000; timer1 base=0x101E2020
// timer3 base=0x101E3000; timer1 base=0x101E3020

int kprintf(char *fmt, ...);
extern int strcpy(char *, char *);
extern int row, col;
int kpchar(char, int, int);
int unkpchar(char, int, int);
int srow, scol;
char clock[16]; 
char *blanks = "  :  :  ";        
int hh, mm, ss;
u32 tick=0;
int oldcolor;

typedef struct tn{
       struct tn *next;
       int    time;
       PROC   *who;
} TNODE;

TNODE tnode[NPROC], *tq, *ft;

extern int irq_stack_top;

void timer0_handler() {

  int ris,mis, value, load, bload, i, spsr;
  int *ip, *iq;
  PROC *p;
    TNODE *tqq;
    oldcolor = color;
    color = GREEN;
    
    ris = tp[0]->RIS;
    mis = tp[0]->MIS;
    value = tp[0]->VALUE;
    load  = tp[0]->LOAD;
    bload=tp[0]->BGLOAD;

    tick++;
    if (tick == 16){
      tick=0; ss++;
    }
    if (ss==60){
      ss = 0; mm++;
    }
    if (mm == 60){
        mm = 0;
	hh++;
    }
    if (tick==0){  // every second
      oldcolor = color;
      color = GREEN;
      for (i=0; i<8; i++){
          unkpchar(clock[i], 0, 70+i);
      }
  
      clock[7]='0'+(ss%10); clock[6]='0'+(ss/10);
      clock[4]='0'+(mm%10); clock[3]='0'+(mm/10);
      clock[1]='0'+(hh%10); clock[0]='0'+(hh/10);

      for (i=0; i<8; i++){
        kpchar(clock[i], 0, 70+i);
      }
       // process sleepers
       for (i=1; i<NPROC; i++){
         p = &proc[i];
	 if (p->status==SLEEP && p->event==PAUSE){
	   p->pause--;
           printf("%d", p->pause);
           if (p->pause <= 0){
 	     //can't kwakeup(PAUSE) because this would wakeup all on PAUSE
             // must take p out of sleepList and make it READY
             // kwakeup(PAUSE);

             outSleep(p);
             p->status = READY;
             p->event = 0;
             p->priority = 128;
	     enqueue(&readyQueue, p);
             //schedule(p);
             printf("wakeup %d\n", p->pid);
           }
	 }
       }  

      // at each second process timerQ
      /* processing timer queue elements */
      if (tq){ // do these only if tq not empty
	  printTQ();
          tqq = tq;
          while (tqq){
             tqq->time--;
             if (tqq->time <= 0){ // send SIGALRM = 14
	       printf("send signal 14 to %d ", tqq->who->pid);
                 tqq->who->res->signal |= (1 << 14);
		 // printf("signal=%d\n", tqq->who->res->signal);
			       
                 tq = tqq->next;
                 put_tnode(tqq);
                 tqq = tq;
                 //printTQ();
             }
             else{
                   break;
             }
          }
       }
    }
    // at each tick  
    if (running->pid)    // excluding P0
       running->time--;

    spsr = get_spsr();
    /************
    if (running->time<=0){
      //printf("%d time up inUmode=%d\n", running->pid, inUmode());
        swflag = 1;
    }
    ************/

  timer_clearInterrupt(0);
  color = oldcolor;

  if (running->time <= 0 && (spsr & 0x1F)== 0x10 && readyQueue->pid){
    printf("%d timer switch process to %d\n", running->pid, readyQueue->pid);
    swflag = 1;
  }
    
  return;
}

void timer_init()
{
  int i;
  kprintf("timer_init() ");

  // set timer base address
  tp[0] = (TIMER *)(0x101E2000); 
  tp[1] = (TIMER *)(0x101E2020);
  tp[2] = (TIMER *)(0x101E3000); 
  tp[3] = (TIMER *)(0x101E3020);
 
 // set control counter regs to defaults
  for (i=0; i<4; i++){
    tp[i]->LOAD = 0x0;   // reset
    tp[i]->VALUE= 0xFFFFFFFF;

    //tp[i]->VALUE= 0x000000FF;
    tp[i]->RIS  = 0x0;
    tp[i]->MIS  = 0x0;
    tp[i]->LOAD    = 0x100;
    tp[i]->CONTROL = 0x62;  // 011- 0000=|NOTEn|Pe|IntE|-|scal=00|32-bit|0=wrap|
    //tp[i]->BGLOAD  = 0xF0000; // about per second interrupt
    tp[i]->BGLOAD    = 0xF000;

  }
  strcpy(clock, "00:00:00");
  hh = mm = ss = 0;

  ft = &tnode[0]; 
  for (i=0; i<NPROC; i++)
    tnode[i].next = &tnode[i+1];
  tnode[NPROC-1].next=0;
  tq = 0;

  
}

void timer_start(int n) // timer_start(0), 1, etc.
{
  TIMER *tpr;
  kprintf("timer_start\n");
  tpr = tp[n]; 
  tpr->CONTROL |= 0x80;  // set enable bit 7
}
int timer_clearInterrupt(int n) // timer_start(0), 1, etc.
{

  TIMER *tpr = tp[n];
  tpr->INTCLR = 0xFFFFFFFF;
}

void timer_stop(int n) // timer_start(0), 1, etc.
{
  TIMER *tptr = tp[n];
  tptr->CONTROL &= 0x7F;  // clear enable bit 7
}
TNODE *get_tnode()
{
    TNODE *tp;
    tp = ft;
    ft = ft->next;
    return tp;
}

int put_tnode(TNODE *tp)
{
    tp->next = ft;
    ft = tp;
}

int printTQ()
{
   TNODE *tp;
   tp = tq;

   printf("timer = ");

   while(tp){
      printf(" [%d, %d] ==> ", tp->who->pid,tp->time);
      tp = tp->next;
   }
   printf("\n");
}

kitimer(int time)
{
    TNODE *t, *p, *q;
    int ps;

    // CR between clock and this process
    ps = int_off();
    t = get_tnode();
    t->time = time;
    t->who = running;

    /******** enter into tq ***********/
    if (tq==0){
        tq = t;
        t->next = 0;
    }
    else{
          q = p = tq;
          while (p){ 
              if (time - p->time < 0) 
                  break;  
              time -= p->time;
              q = p;
              p = p->next;
          }
          if (p){ 
              p->time -= time;
          }
          t->time = time;
          if (p==tq){
              t->next = tq;
              tq = t;
          }
          else{
                t->next = p;
                q->next = t;
          }
    }

    //running->status = TIMER;
    int_on(ps);

    printTQ();
    // printf("%d exit kitimer\n", running->pid);
    //    tswitch();
    // return to umode ==> will get SIGALRM when timer expires
}
