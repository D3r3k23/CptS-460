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

#define enterCR  ps = int_off()
#define exitCR   int_on(ps)

extern PROC *readyQueue;

void P(struct semaphore *s)
{
  int ps;

  ps = int_off();

   s->value--;
   if (s->value < 0){
     //printf("P: block %d\n", running->pid); 
      running->status=BLOCK;
      running->sem = s;       /* PROC's sem pointer->this semaphore */
      enqueue(&s->queue, running);
      tswitch();
   }
   int_on(ps);
}

void V(struct semaphore *s)
{
  PROC *p; int ps;
   
    ps=int_off();
    s->value++;
    if (s->value <= 0){
        p = (PROC *)dequeue(&s->queue);
        p->status = READY;
        enqueue(&readyQueue, p);
	// printf("V: unblock %d\n", p->pid);
    }
    int_on(ps);
}

int wV(PROC *p)
{
  PROC *q, *r;
  int ps;
  struct semaphore *s = p->sem;

  ps = int_off();

  /*********** debugging *******
  q = s->queue;
  while(q){
    printf("%d->", q->pid);
    q = q->next;
  }
  ****************************/ 
    if (p == s->queue){
       s->queue = p->next;
    }
    else{
      r = s->queue;
      q = r->next;
      while(q){
        if (p == q){
	  r->next = q->next;
          break;
        }
        r = q;
        q = q->next;
      }
    }
    s->value++;
    p->status = READY; p->sem = 0;
    enqueue(&readyQueue, p);
    printf("unblocked %d\n", p->pid);
  int_on(ps);
}

