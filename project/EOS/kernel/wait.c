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

int enterSleep(PROC *p)
{
  PROC *q;
  if (sleepList == 0)
    sleepList = p;
  else{
    q = sleepList;
    while (q->next)
       q = q->next;
    q->next = p;
  }
  p->next = 0;
}

PROC *outSleep(PROC *ptr)
{
  PROC *p, *q; 
  p = q = sleepList;

  while(p){
    if (p->pid != ptr->pid){
      q = p;
      p = p->next;
      continue;
    }
    // found ptr at p
    if (p==sleepList){      // if first in sleepList
	 sleepList = p->next;
    }
    else{                   // not first in sleepList   
       q->next = p->next;   // delete p from list
    }
    return p;
  }
}

int ksleep(int event)
{
  //printf("proc %d ksleep on %x\n", running->pid, event);  
  int sr = int_off();

  running->event = event;
  running->status = SLEEP;
  enqueue(&sleepList, running);
  //printSleepList(sleepList);
  tswitch();

  int_on(sr);
}

int kwakeup(int event)
{
  PROC *p, *tmp=0;
  int sr = int_off();

  while((p = dequeue(&sleepList))!=0){
    if (p->event==event){
      //printf("\nkwakeup %d\n", p->pid);
      p->status = READY;
      enqueue(&readyQueue, p);
    }
    else{
      enqueue(&tmp, p);
    }
  }
  sleepList = tmp;

  int_on(sr);
}

int texit(int value)
{
  printf("proc %d in texit, exitValue=%x\n", running->pid, value);
 
  running->exitCode = value;
  running->status = ZOMBIE;
  kwakeup((int)running->parent);

  tswitch();
}

int kexit(int value)
{
  int i; PROC *p; int wk1;
  wk1 = 0;

  //printf("proc %d in kexit, exitValue=%x\n", running->pid, value);
  if (running->pid==1){
     kprintf("P1 never dies\n");
     return -1;
  }

  /* close opened file descriptors: so far only pipes*/
  for (i=0; i<NFD; i++){
      if (running->res->fd[i] != 0)
	//close_pipe(i);
        myclose(i);
  }

    // release PROC's cwd
    ilock(running->res->cwd);
    iput(running->res->cwd);

    // clear PROC's signals  
    for (i=0; i<NSIG; i++)
        running->res->sig[i] = 0;
    running->res->signal = 0;

  for (i=1; i<NPROC; i++){
    p = &proc[i];
    if ((p->status != FREE) && (p->ppid == running->pid)){
      //printf("give %d to P1\n", p->pid);
      p->ppid = 1;
      p->parent = &proc[1];
      wk1++;
    }
  }
  running->exitCode = value;
  running->status = ZOMBIE;
  kwakeup((int)running->parent);
  if (wk1)
     kwakeup((int)&proc[1]);
  tswitch();
}

 int kwait(int *status)
 {
   int i; PROC *p;
   int child = 0;
   //printf("proc %d in kwait() : ", running->pid);
   for (i=1; i<NPROC+NTHREAD; i++){
     p = &proc[i];
     if (p->status != FREE && p->ppid == running->pid){
       child++;
     }
   }
   if (child==0){
     printf("no child\n");
     return -1;
   }
   
   while(1){
      for (i=1; i<NPROC+NTHREAD; i++){
	p = &proc[i];
        if ((p->status==ZOMBIE) && (p->ppid == running->pid)){
	  //kprintf("proc %d found a ZOMBIE child %d ", running->pid,p->pid);
           *status = p->exitCode;
	   p->status = FREE;
	   if (p->type == THREAD)
	     tputproc(p);
	   else
             putproc(p);
	     // printf("%d exit kwait\n", running->pid);
	   return p->pid;
        }
      }
      //printf("sleep on %x\n", running);
      ksleep((int)running); 
   }   
}



