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

// queue.c file
#include "../type.h"

extern PROC *freeList;
extern PROC *tfreeList;

PROC *getproc()
{
  PROC *p = freeList;
  if (p){
    freeList = p->next;
  }
  return p;
}

int putproc(PROC *p) // release to END of freeList
{
  /*
  p->status = FREE;
  p->next = freeList;
  freeList = p;
  */

  PROC *q = freeList;
  if (freeList == 0){
     p->next = freeList;
     freeList = p;
  }
  else{
    while(q->next)
      q = q->next;
    q->next = p;
    p->next = 0;
  }

}

PROC *tgetproc()
{
  PROC *p = tfreeList;
  
  if (p){
    tfreeList = p->next;
  }
  return p;
}

int tputproc(PROC *p) // release to END of freeList
{
  p->status = FREE;
  p->next = tfreeList;
  tfreeList = p;

  /*    
  PROC *q = freeList;
  if (freeList == 0){
     p->next = freeList;
     freeList = p;
  }
  else{
    while(q->next)
      q = q->next;
    q->next = p;
    p->next = 0;
  }
  */
}

int enqueue(PROC **queue, PROC *p)
{
  PROC *q  = *queue;
  if (q==0 || p->priority > q->priority){
    *queue = p;
    p->next = q;
    return;
  }
  while (q->next && p->priority <= q->next->priority){
    q = q->next;
  }
  p->next = q->next;
  q->next = p;
}

PROC *dequeue(PROC **queue)
{
  PROC *p = *queue;
  if (p)
    *queue = p->next;
  return p;
}

int printQ(PROC *p)
{
  kprintf("readyQueue = ");
  while(p){
    kprintf("[%d%d]->", p->pid,p->priority);
    p = p->next;
  }
  kprintf("NULL\n");
}
int printQueue(PROC *p)
{
  kprintf("readyQueue = ");
  while(p){
    kprintf("[%d%d]->", p->pid,p->priority);
    p = p->next;
  }
  kprintf("NULL\n");
}

int printSleepList(PROC *p)
{
  printf("sleepList  = ");
   while(p){
     kprintf("[%d%d]->", p->pid, p->event);
     p = p->next;
  }
  kprintf("NULL\n"); 
}

int printList(PROC *p)
{
   kprintf("freeList   = ");
   while(p){
     kprintf("[%d]->", p->pid);
     p = p->next;
  }
  kprintf("NULL\n"); 
}
