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

/********************
#define  SSIZE 1024
#define  NPROC 64
#define  FREE   0
#define  READY  1
#define  SLEEP  2
#define  BLOCK  3
#define  ZOMBIE 4
#define  printf  kprintf
 
typedef struct proc{
  struct proc *next;
  int    *ksp;
  int    *usp;
  int    *upc;
  int    *cpsr;
  int    inkmode;

  int    status;
  int    priority;
  int    pid;
  int    ppid;
  struct proc *parent;
  int    event;
  int    exitCode;

  char   name[32];
  PRES   *res;
  int    kstack[SSIZE];
}PROC;
***************************/

extern PROC *kfork();

PROC proc[NPROC+NTHREAD], *freeList, *readyQueue, *sleepList, *running;
PRES pres[NPROC];

volatile int swflag;
PROC *tfreeList;

int procsize = sizeof(PROC);
char *pname[NPROC]={"sun", "mercury", "venus", "earth", "mars", "jupiter",
                     "saturn","uranus","neptune"};
OFT  oft[NOFT];
PIPE pipe[NPIPE];

char line[8];

#define MTABLE  0x4000;       // initial ptable at 16KB
#define UTABLE  0x500000;     // porc umode ptales are in 4MB-6MB
/********************** MTX memory map ******************************
0-2M: kernel
3-4M: fbuf of LCD display
5-6M: porc umode page tables, each 4*4096=16KB => 2M/16K=128 porcs
7-8M: I/O buffers
8-256M: user images; 1M per proc from p1 to p128 (P0's ptabel at 0x4000)
***********************************************************************/
int init()
{
  int i, j; 
  PROC *p; char *cp;
  int *Mtable, *mtable;
  int paddr;

  kprintf("kernel_init()\n");
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->status = FREE;
    p->priority = 0;
    p->ppid = 0;
 
    p->res = &pres[i];   // res point to pres[i]
    // p->usp = &ustack[i][1024]; // ustack is at the high end of Uimage
    kstrcpy(p->res->name, pname[i]);
    p->next = p + 1;
    // proc[i]'s umode page dir and pagetable is at 5M + pid*16KB
    p->res->pgdir = (int *)(0x500000 + p->pid*0x4000); // must be 16KB boundary
  }
  proc[NPROC-1].next = 0;
  freeList = &proc[0];
  //printList(freeList);

  for (i=NPROC; i<NPROC+NTHREAD; i++){
    p = &proc[i];
    p->pid = i;
    p->status = FREE;
    p->priority = 0;
    p->ppid = 0;
    p->next = p + 1;
  }
  proc[NPROC+NTHREAD-1].next = 0;
  tfreeList = &proc[NPROC];
  //printList(tfreeList);

  readyQueue = 0;
  sleepList = 0;
 
  // creat P0 as running;
  p = running = getproc();
  p->status = READY;
  p->inkmode = 1;

  p->res->uid = p->res->gid = 0;    
  p->res->signal = 0;
  p->res->name[0] = 0;
  p->time = 10000;              // arbitray since P0's p time never decreases

  for (i=0; i<NFD; i++)
     p->res->fd[i] = 0;
  for (i=0; i<NSIG; i++)
    p->res->sig[i] = 1;   // P0 ignore all signals

  // USE 1-level paging of 1MB sections
  printf("building pgdirs at 5MB\n");
  // create pgdir's for ALL PROCs at 3MB; Mtable at 0x4000 by hardcode in ts.s
  Mtable = (int *)MTABLE;     // Mtable at 0x4000
  mtable = (int *)UTABLE;     // proc mtables begin at 4MB

  // Each pgdir MUST be at a 16K boundary ==>
  // 5MB to 6MB has space for 64 pgdirs for 64 PROCs
  for (i=0; i<NPROC; i++){    // for 64 PROC mtables
    for (j=0; j<2048; j++){
       mtable[j] = Mtable[j]; // copy low 2048 entries of Mtable
    }
    mtable += 4096;           // advance mtable to next 16KB
  }
  mtable = (int *)UTABLE;      // mtables begin at 5MB
  for (i=0; i<NPROC; i++){
    for (j=2049; j<4096; j++){ // zero out high 2048 entries
      mtable[j] = 0;
    }
    //proc[0] does not need a mtable, proc[i]'s pagetable: VA=0x800000 + 
    mtable[2048]=(0x800000 + (i-1)*0x100000)|0xC12; // entry 2048 OR in 0xC12  
    mtable += 4096;
  }

  binit();
  pipe_init();

  fs_init();
  // after mount_root, must set P0's CWD
  running->res->cwd = root;

  thinit();
}

int scheduler()
{
  char line[8];
  int pid; PROC *old=running;
  char *cp;
  //  if (running->pid)
  //  kprintf("proc %d in scheduler  ", running->pid);
  if (running->status==READY)
     enqueue(&readyQueue, running);
  //if (running->pid)
  //   printQ(readyQueue);
  running = dequeue(&readyQueue);
  //if (running->pid)
  //   printQ(readyQueue);
  //if (running->pid)
  // kprintf("next running = %d\n", running->pid);

  color = running->pid % 7;

  // must switch to new running's pgdir; possibly need also flush TLB
  if (running->res->pgdir != old->res->pgdir){ // for threads NOT in same VA
     switchPgdir((u32)running->res->pgdir);
  }
  if (running->type==THREAD){
    setulr(VA(4));
  }
  running->time = 8;  // 8 ticks seconds
  swflag = 0;
}

extern int irq_tswitch();

int reschedule()
{
  int mode = get_cpsr() & 0x1F;
  if (swflag){
    if (mode == 0x13){ // SVC mode
       tswitch();
    }
    else if (mode == 0x12){ // IRQ mode
      irq_tswitch();
    }
  }
}
