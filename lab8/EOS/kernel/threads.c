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

/*************************************************************************
      threads.c : implement thread(), texit(), and  mutex
*************************************************************************/
#include "../type.h"

extern PROC *tfreeList;
extern int goUmode();

#define TMAX 16

int thinit()
{
   int i;
   PROC *p;
   printf("thinit()\n");
   for (i=NPROC; i<NPROC+NTHREAD; i++){
     p = &proc[i];
     p->pid = i;
     p->status = FREE;
     p->priority = 0;
     p->next = p+1;
     p->proc = p;   // each PORC points to itself
     p->inkmode = 1;
     p->type = THREAD;
     p->res = 0;
   }
   tfreeList = &proc[NPROC];
   proc[NPROC+NTHREAD-1].next = 0;
}

int ktjoin(int n)
{
  int i, *status;
  printf("proc %d in ktjoin n=%d\n", running->pid, n);
  for (i=0; i<n; i++)
     kwait(&status);
  printf("proc %d exits ktjoin\n", running->pid);
}

// no need for texit(); from U mode same as exit()
int ktexit(int v)
{
    kexit(v);
}

#define USSIZE 1024
#define ENDPC  0x05

int kthread(int fn, int stack, int ptr)
{  
  // create a thread to execute fn with stack; return thread's pid
  // flag = 1 : use ustack by thread pid ustack=tid9 tid10 .... tidTMAX
  // flag = 0 : use passed in ustack
  PROC *p, *q;
  int i, j, *ip, *iq, tcount, index;
  u32 offset, segment;

  //  printf("kernel thread(): fn=%x stack=%x ptr=%x\n", fn, stack, ptr);

  printf("kernel thread(): fn=%x stack=%x ptr=%x\n", fn, stack, ptr);

  //check process tcount
  p = running->proc; 
  tcount = p->res->tcount;
  printf("process %d tcount = %d\n", running->pid, tcount);
        
  if (tcount > TMAX){
      printf("max proc tcount %d reached\n", tcount);
      return -1;
  }
  
  segment = running->res->pgdir[2048];

  
  // 1. need a new thread sturct
  /*** get a proc for child thread: ***/
  if ( (p = (PROC *)tgetproc(&tfreeList)) == 0){
       printf("\nno more THREAD PROC  ");
       return(-1);
  }
  /* initialize the new thread PROC */
  printf("pid = %d ", p->pid);
  p->status = READY;
  p->ppid = running->pid;
  p->parent = running;
  p->priority  = 1;
  p->event = 0;
  p->exitCode = 0;
  p->inkmode = 1;
  p->cpu = 0;
  p->type = THREAD;

  //  p->proc = running->proc;       // point to process PROC
  //p->res  = running->proc->res;  // point to SAME resource struct of PROCess
  p->proc = running;       // point to process PROC
  p->res  = running->res;  // point to SAME resource struct of PROCess

  printf("pgdir=%x ", p->res->pgdir);
  /*************
  // if flag=1: based on pid of thread, use ustack accordingly
  if (flag==1){
     // stack is very high end of a total ustack area; each thread
     //   uses its portion of the ustack area 
     index = p->pid - NPROC;            // 9-9=0 10-9=1, etc
     printf("index=%d ", index);
     stack -= (index*USSIZE);         // assume 1024 bytes ustack per thread 
     //kgetc();
  }
  ***************/
  p->usp = stack + 1024;
  p->cpsr = running->cpsr;

  for (i=1; i<29; i++)
       p->kstack[SSIZE-i] = 0;

 // kstack must contain a resume frame FOLLOWed by a goUmode frame
  //  ksp  
  //  -|-----------------------------------------
  //  r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 fp ip pc|
  //  -------------------------------------------
  //  28 27 26 25 24 23 22 21 20 19 18  17 16 15
  //  
  //   usp      NOTE: r0 is NOT saved in svc_entry()
  // -|-----goUmode--------------------------------
  //  r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 ufp uip upc|
  //-------------------------------------------------
  //  14 13 12 11 10 9  8  7  6  5  4   3    2   1
  
  p->upc = (int *)fn;                // NEED THIS in goUmode()

  p->kstack[SSIZE-1] = (int)fn;      // same as upc
  p->kstack[SSIZE-14] = (int)ptr;    // ptr is saved R0
  p->kstack[SSIZE-15] = (int)goUmode;  // in dec reg=address ORDER !!!
  p->ksp = &(p->kstack[SSIZE-28]);
  
  // -|-----goUmode-------------------------------------------------
  //  r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 ufp uip upc|string       |
  //----------------------------------------------------------------
  //  14 13 12 11 10 9  8  7  6  5  4   3    2   1 |             |

  p->status = READY;   

  enqueue(&readyQueue, p);
  printQueue(readyQueue);
 
  // update total tcount in process PROC
  running->proc->res->tcount++;
  printf("proc %d created a thread %d in segment=%x tcount=%d\n",
           running->pid, p->pid, segment, p->proc->res->tcount);
  //   ntasks++;
   kgetc();
   return(p->pid);
}
int kkthread(int fn, int stack, int flag, int ptr)
{  
  // create a thread to execute fn with stack; return thread's pid
  // flag = 1 : use ustack by thread pid ustack=tid9 tid10 .... tidTMAX
  // flag = 0 : use passed in ustack
  PROC *p, *q;
  int i, j, *ip, *iq, tcount, index;
  u32 offset, segment;

  //  printf("kernel thread(): fn=%x stack=%x ptr=%x\n", fn, stack, ptr);

  printf("kernel thread(): fn=%x stack=%x ptr=%x\n", fn, stack, ptr);

  //check process tcount
  p = running->proc; 
  tcount = p->res->tcount;
  printf("process %d tcount = %d\n", running->pid, tcount);
        
  if (tcount > TMAX){
      printf("max proc tcount %d reached\n", tcount);
      return -1;
  }
  
  segment = running->res->pgdir[2048];

  
  // 1. need a new thread sturct
  /*** get a proc for child thread: ***/
  if ( (p = (PROC *)tgetproc(&tfreeList)) == 0){
       printf("\nno more THREAD PROC  ");
       return(-1);
  }
  /* initialize the new thread PROC */
  printf("pid = %d ", p->pid);
  p->status = READY;
  p->ppid = running->pid;
  p->parent = running;
  p->priority  = 1;
  p->event = 0;
  p->exitCode = 0;
  p->inkmode = 1;
  p->cpu = 0;
  p->type = THREAD;

  //  p->proc = running->proc;       // point to process PROC
  //p->res  = running->proc->res;  // point to SAME resource struct of PROCess
  p->proc = running;       // point to process PROC
  p->res  = running->res;  // point to SAME resource struct of PROCess

  printf("pgdir=%x ", p->res->pgdir);

  // if flag=1: based on pid of thread, use ustack accordingly
  if (flag==1){
     // stack is very high end of a total ustack area; each thread
     //   uses its portion of the ustack area 
     index = p->pid - NPROC;            // 9-9=0 10-9=1, etc
     printf("index=%d ", index);
     stack -= (index*USSIZE);         // assume 1024 bytes ustack per thread 
     //kgetc();
  }
  
  p->usp = stack + 1024;
  p->cpsr = running->cpsr;

  for (i=1; i<29; i++)
       p->kstack[SSIZE-i] = 0;

 // kstack must contain a resume frame FOLLOWed by a goUmode frame
  //  ksp  
  //  -|-----------------------------------------
  //  r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 fp ip pc|
  //  -------------------------------------------
  //  28 27 26 25 24 23 22 21 20 19 18  17 16 15
  //  
  //   usp      NOTE: r0 is NOT saved in svc_entry()
  // -|-----goUmode--------------------------------
  //  r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 ufp uip upc|
  //-------------------------------------------------
  //  14 13 12 11 10 9  8  7  6  5  4   3    2   1
  
  p->upc = (int *)fn;                // NEED THIS in goUmode()

  p->kstack[SSIZE-1] = (int)fn;      // same as upc
  p->kstack[SSIZE-14] = (int)ptr;    // ptr is saved R0
  p->kstack[SSIZE-15] = (int)goUmode;  // in dec reg=address ORDER !!!
  p->ksp = &(p->kstack[SSIZE-28]);
  
  // -|-----goUmode-------------------------------------------------
  //  r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 ufp uip upc|string       |
  //----------------------------------------------------------------
  //  14 13 12 11 10 9  8  7  6  5  4   3    2   1 |             |

  p->status = READY;   

  enqueue(&readyQueue, p);
  printQueue(readyQueue);
 
  // update total tcount in process PROC
  running->proc->res->tcount++;
  printf("proc %d created a thread %d in segment=%x tcount=%d\n",
           running->pid, p->pid, segment, p->proc->res->tcount);
  //   ntasks++;
   //kgetc();
   return(p->pid);
}

// implementation of  mutex

typedef struct{
  int status;     // created OR destroyed
  int owner;     // owner pid, only owner can unlcok
  int value;     // lock value: 0 for locked 1 for FREE
  PROC *queue;   // waiting proc queue; ==> unlock will wakeup one as owner 
}MUTEX;

MUTEX  mutex[10];
int mutexuse[10] = {0};  // all mutex[] are FREE

int kmutex_creat() // no need for value because initial always 1==FREE
{
  int i;
  for (i=0; i<10; i++){
    if (mutexuse[i]==0){
      mutexuse[i] = 1;
      break;
    }
  }
  // allocated mutex[i];
  mutex[i].status = 1;
  mutex[i].value = 1;
  mutex[i].queue = 0;
  mutex[i].owner = -1;   // no owner initially
  printf("\nkernel: mutex_creat(): mutex==%x  mutex->value=%d\n",
          &mutex[i], mutex[i].value);
  return (int)&mutex[i];
}   

int kmutex_destroy(MUTEX *m)
{
  int i;
  printf("mutex_destroy : ");

  // owner ==> deallocate mutex m
  if (m->owner != running->pid){
    printf("not owner\n");
    return -1;
  }

  if (m->value < 0){
    printf("mutex still has waiters\n");
    return -1;
  }
  for (i=0; i<10; i++){
    if (m==&mutex[i]){
      mutexuse[i] = 0;
      mutex[i].status = 0;
      mutex[i].owner = -1;
    }
  }
  printf("destroyed\n");
}

int kmutex_lock(MUTEX *m)
{
  int i;
  int found = 0;
  PROC *p;
  
  printf("\nmutex_lock : ");

  for (i=0; i<10; i++){
    if (m==&mutex[i]){
      found = 1;
      break;
    }
  }

  if (!found){
      printf("invalid mutex %x\n", m);
      return -1;
  }

  if (m->status == 0){
    printf("invalid mutex\n");
    return -1;
  }

  if (m->value <= 0 && m->owner==running->pid){
    printf("mutex is already locked by you!\n");
    return -1;
  }

  m->value--;

  if (m->value < 0){
    // block caller in mutex queue   

    p = m->queue;
    if (m->queue==0)
      m->queue = running;
    else{
      while(p->next)
	p = p->next;
      p->next = running;
    }
    p->next = 0;
    running->status = BLOCK;
    printf("%d BLOCKed in mutex %x queue\n", running->pid, m);
    tswitch();
  }

  m->owner = running->pid;
  printf("Proc%d locked mutex=%x\n", running->pid, m);
  return 0;
}

int kmutex_unlock(MUTEX *m)
{
  int i;
  int found = 0;
  PROC *p;

  printf("\nmutex_unlock : ");

  for (i=0; i<10; i++){
    if (m==&mutex[i]){
      found = 1;
      break;
    }
  }

  if (!found){
      printf("invalid mutex %x\n", m);
      return -1;
  }

  if (m->status == 0){
    printf("invalid mutex\n");
    return -1;
  }


  if (m->owner != running->pid){
    printf("%d is NOT owner of this mutex\n", running->pid);
    return -1;
  }

  if (m->value > 0){
    printf("mutex %x is NOT locked\n", m);
    return -1;
  }
  
  m->value++;
  if (m->value <= 0){
     p = m->queue;
     m->queue = p->next;
     m->owner = p->pid;
     p->status = READY;

     enqueue(&readyQueue, p);
     printf("UNBLOCK %d=new owner\n", p->pid);
  }
}

