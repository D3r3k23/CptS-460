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
extern PROC proc[ ];
extern int goUmode();

extern PROC *freeList, *tfreeList;

#define TMAX 16

int thinit()
{
   int i;
   PROC *p;

   for (i=0; i<NTHREAD; i++){
     p = &proc[NPROC+i];
     p->pid = i + NPROC;
     p->status = FREE;
     p->priority = 0;
     p->next = p+1;
     p->parent = p;        // each tPORC points to itself
     p->type = THREAD;
     p->tcount = 0;
   }
   tfreeList = &proc[NPROC];
   proc[NPROC+NTHREAD-1].next = 0;
}


int ktjoin(int n)
{
  int i, status;
  for (i=0; i<n; i++){
    printf("%d in ktjoin\n", running->pid);
     kwait(&status);
  }
}

// no need for texit(); from U mode same as exit()
int ktexit(int v)
{
  texit(v);
}

#define USSIZE 1024

int kthread(int fn, char *stack, int *ptr)
{  
  // create a thread executing fn with stack; return thread's pid
  PROC *p;
  int i, uaddr, tcount; 
  int index;
  char *ustack;
  
  printf("kernel thread(): fn=%x  stack=%x ptr=%x *ptr=%d\n", 
          fn, stack, ptr, *ptr);
  
  uaddr = running->res->pgdir[2048]&0xFFFF0000;

  // check process tcount
  tcount = running->tcount;
  //printf("process %d tcount = %d\n", running->pid, tcount);
  if (tcount > TMAX){
      printf("max proc tcount %d reached\n", tcount);
      return -1;
  }
  // 1. need a new thread sturct
  /*** get a proc for child thread: ***/

  p = (PROC *)tgetproc();  // get a PROC from tfreeList
  if (p == 0){ 
     printf("\nno more THREAD PROC  ");
     return -1;
  }
  /* initialize the new thread PROC */
  p->status = READY;
  p->ppid = running->pid;
  p->parent = running;
  p->priority  = 1;
  p->event = 0;
  p->exitCode = 0;

  // stack passed in is the beginning of a total stack area
  // each thread uses a section of the total stack area based on its pid
     index = p->pid - NPROC;            // 9-9=0 10-9=1, etc
     printf("index=%d ", index);
     ustack = stack + index*USSIZE;    // assume 1024 bytes ustack per thread 

  //p->pgdir = running->pgdir; // same page table as parent
  p->proc = running;   // point to running
  p->res = running->res;

  p->res->pgdir = running->res->pgdir;
  
  p->type = THREAD;
  p->tcount = 0;
  p->parent = running; // p->proc = running->proc;   // point to process PROC

  p->cpsr = running->cpsr;
  p->usp = ustack + 1024; // high end of 1KB ustack area 
  for (i=1; i<29; i++)
     p->kstack[SSIZE-i] = 0;

  p->upc = fn;
  p->kstack[SSIZE-1] = fn; 
  p->kstack[SSIZE-14] = (int)ptr;
  p->kstack[SSIZE-15] = (int)goUmode;
  p->ksp = &p->kstack[SSIZE-28];

  enqueue(&readyQueue, p);
  printQ(readyQueue);
  running->tcount++;
  printf("proc %d created a thread %d in PA=%x tcount=%d\n",
         running->pid, p->pid, uaddr, running->tcount);
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

