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

/******************* signals.c *******************************/
#include "../type.h"
extern u32 irq_stack_top;

// deliver signal to pid: what if pid is SLEEP or BLOCK?
// SLEEP ==> wakeup  BLOCK==> must V the semaphore

/*********************** KCW: 4-20-06 ************************************  
 kill delivers a signal to a process. If the process is BLOCK or SLEEP,
 we should V()/wakeup it.
 If the process is BLOCKed on a sempahore, we must know which semaphore it is 
 blocked on. So, if a proc is blocked on a semaphore, we let proc.sem -> the
 blocking semaphore. Then we can unblock the process from the semaphore queue
 by the wV(running) operation.
***************************************************************************/
extern int kbhead, kbtail;
UART stty[2];
extern struct semaphore kbData;
extern UART uart[4];
extern KBD  kbd;

int kkill(int sig, int pid)
{
  // kill sig pid;
  PROC *p;
  int i, oldsig;
  UART *up;
  KBD *kp;
  
  // NOTE: in PMTX and SMP, proc[i] has pid=i+1, so dec pid by 1
  if (pid < 1 || pid > NPROC) return -1;
  if (sig < 1 || sig >= NSIG) return -1;

  p = &proc[pid];
  if (p->status==FREE || p->status == ZOMBIE) 
     return -1;

  /**** uncomment this to allow only superuser to kill **************
  if (running->res->uid != 0){
     if (running->res->uid != p->res->uid) 
        return -1
  }
  *******************************************************************/
  oldsig = running->res->signal;
  p->res->signal = p->res->signal | (1 << sig);  // turn sig-th bit on

  if (p->status==SLEEP){  // wakeup p if it's SLEEP
     kwakeup(p->event);
     kp = &kbd;
     kp->data++; kp->room--;
     kp->head++;
  }

  if (p->status==BLOCK){
    // this task is BLOCKed on p->sem semaphore; it may NOT be the first in
    // that semQ, so must do a different V
    // unblock p only if it is waiting for terminal inputs 
    // printf("sem=%x kbData=%x\n", p->sem, &kbData);
    /*************************** KCW ***************************************
      although the proc is unblocked, its resuming point is still in getc() or
      sgetc(), which would advance the input buffer tail pointer by 1, causing
      the next proc to get wrong keys. There are 2 options: write a dummy char 
      to the input buffer OR decrement tail pointer by 1.
    **********************************************************************/
  
    //printf("sem=%x inch=%x ", p->sem, &uart[0].inchar);
    if (p->sem==&uart[0].inchar || p->sem==&uart[1].inchar){
      //printf("%d was blocked on semaphore\n", p->pid);
      // let dyning proc read a dummy char from input buffer
      if (p->sem == &uart[0].inchar){
         up = &uart[0];
         up->inhead++; 
      }
      if (p->sem == &uart[1].inchar){
	 up = &uart[1];
         up->inhead++;
     }
     wV(p); // unblock proc from semaphore queue and adjust semaphore value
   }
   // servers may be blocked in krecv() on p->res->message semaphore
   if (p->sem)
      wV(p);
  }
  printf("Kernel: killed pid=%d with sig# %d\n", pid, sig);
  return oldsig;
}

int ksignal(int sig, int ucatcher)
{
  int oldsig;

  if (sig < 1 || sig > NSIG) return -1;
  if (sig==9){
      printf("can't change signal 9\n");
      return -1;
  }
  //printf("%d install catcher=%x for signal# %d\n",running->pid,ucatcher,sig);
  oldsig = running->res->sig[sig];
  running->res->sig[sig] = ucatcher;
  //printf("installed catcher=%x for sig%d\n", ucatcher, sig);
  return oldsig;
}

int setsig(int n)
{
  running->res->signal |= (1 << n);
  printf("set signal %dfor %dsignal=%x\n",n,running->pid,running->res->signal);
}

int cksig()
{
  // check pending signal
  int i;
  for (i=1; i<NSIG; i++){
    if (running->res->signal & (1 << i)){
       running->res->signal &= ~(1 << i);
       //printf("csig: found sig=%d\n", i);
      return i;
    }
  }
  return 0;
}
extern setulr();
// when a process is about return to Umode, handle any outstanding signal 
int kpsig()
{
  int  i, n, upc, oldPC, newPC, w;
  int *cp;
  int *sp, cpsr, mode;
  
  n = cksig();
  if (n==0) return;
  //if (running->pid)
  // printf("kpsig: %d's sig=%d\n", running->pid, n);

  if (running->res->sig[n] == 1){  // IGNore this signal
     //printf("psig: proc%d ignore signal#%d\n", running->pid, n);
    return;
  }
  if (running->res->sig[n] == 0){
     printf("proc %d dying by signal# %d ", running->pid, n);
     /**************
     if in IRQ mode with a signal: can't exit in IRQ mode because tswitch can
     only be in SVC mode
     ****************/
     mode = get_cpsr() & 0x1F;
     if (mode == 0x13){ // SVC mode call kexit()
       printf("in SVC mode\n");
         kexit(n<<8);
     }
     if (mode == 0x12){ // IRQ mode
       printf("in IRQ mode\n");
       irq_exit(n<<8);
     }
  }

  newPC = running->res->sig[n];
  running->res->sig[n] = 0;           // reset catcher to default

  /***** trap stack: timer: in IRQ mode data in abort mode**
            |ulr|r12|r11 r10 r9 r8 r7 r6 r5 r4 r3 r2 r1 r0| 
             1    2  3    4  5  6  7  8  9  10 11 12 13 14
       replace ulr with catch address at newPC
       set r0 to signal #
       set User mode lr to ulr 
SVC stack
   *******************************************/  
   // if in SVC mode => modify kstack

  cpsr = get_cpsr();
  printf("cpsr=%x\n", cpsr & 0xFF);
  if ((cpsr & 0x1F)==0x13){ // SVC mode
     oldPC = running->kstack[SSIZE-1];
     running->kstack[SSIZE-1] = newPC;
     running->upc = newPC;
     running->kstack[SSIZE-14] = n;  // saved r0=n
     setulr(oldPC);
     printf("SVC: oldPC=%x  newPC=%x\n", oldPC, newPC);
  }

  if ((cpsr & 0x1F)==0x12){ // IRQ mode => change IRQ stack
    sp = (int *)&irq_stack_top;
    oldPC = *(sp-1);
    *(sp-1) = newPC;
    *(sp-14) = n;
    setulr(oldPC);
    printf("IRQ: oldPC=%x  newPC=%x\n", oldPC, newPC);
  }	
   // if in IRQ mode => must mdify IRQ stack at irq_stack_top

   return 0;
}
    /* ====================== 32-bit mode ============================
 HI                         kstack                                   LOW
                         
      |uss|usp|uflag|ucs|upc|ax|bx|cx|dx|bp|si|di|uds|ues|ufs|ugs|
            |            ***                                                    
            V                                                        LOW
    ------------------------------------------------------------
       uframes|sig#|oldPC|
    ------------------|---------------------------------------
                     usp

//********************* exception kstack layout ****************************
 |oss|osp|eflag|cs|eip|0/err#|nr|ax|cx|dx|bx|esp|bp|si|di|ds|es|fs|gs|ss|
 |<----- by exception ------>|  |<----- by pushal ------>|
   1   2   3     4  5    6     7  8  9 10 11  12 13 14 15 16 17 18 19 20 

     1. replace upc in kstack by newPC --> ucatcher()
     2. create 2 slots in ustack as oldPC,sig#
     3. usp in kstack -= 2 slots for handler(int sig#)
        when ucatcher() returns, it retuns to oldPC
     ******************************************************/
     /*
     oldPC = running->kstack[SSIZE-5];
     running->kstack[SSIZE-5] = newPC;

     *(u32 *)(running->kstack[SSIZE-2] - 4) = n;
     *(u32 *)(running->kstack[SSIZE-2] - 8) = oldPC;
     running->kstack[SSIZE-2] -= 8;  
     printf("newPC=%x oldPC=%x\n", newPC, oldPC);
     */
     /****** for debugging during development ************
     printf("proc %d :", running->pid);
     cp = (int *)&running->kstack[SSIZE];
     printf("stack hi=%x\n",cp);

     for (i=1; i<=8; i++)
        printf("|%x ", *(cp-i));
     printf("\n-----------------------------\n");
     for (i=9; i<=20; i++)
        printf("|%x ", *(cp-i));
     printf("\n-----------------------------\n");
     printf("%d in kpsig about to exit kmode\n", running->pid);
     *******************************************************/

