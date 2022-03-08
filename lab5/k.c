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

/********************
#define  SSIZE 1024
#define  NPROC  9
#define  FREE   0
#define  READY  1
#define  SLEEP  2
#define  BLOCK  3
#define  ZOMBIE 4
#define  printf  kprintf
 
typedef struct proc{
  struct proc *next;
  int    *ksp;       // offset=4

  int    *usp;       // offset=8
  int    *cpsr;      // offset=12
  int    *upc;       // offset=16

  int    *pgdir;     // level-1 pagetable    

  int    status;
  int    priority;
  int    pid;
  int    ppid;
  struct proc *parent;
  int    event;
  int    exitCode;
  char   name[32];
  int    kstack[SSIZE];
}PROC;
***************************/
extern int kfork();
PROC proc[NPROC], *freeList, *readyQueue, *sleepList, *running;

int procsize = sizeof(PROC);

char *pname[NPROC]={"sun", "mercury", "venus", "earth", "mars", "jupiter",
                     "saturn","uranus","neptune"};

u32 *MTABLE = (u32 *)0x4000;
int kernel_init()
{
  int i, j; 
  PROC *p; char *cp;
  int *MTABLE, *mtable;
  int paddr;

  kprintf("kernel_init()\n");
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->status = FREE;
    p->priority = 0;
    p->ppid = 0;

    strcpy(p->name, pname[i]);
    p->next = p + 1;
    p->pgdir = (int *)(0x600000 + p->pid*0x4000); // must be on 16KB boundary
  }
  proc[NPROC-1].next = 0;
  freeList = &proc[0];
  readyQueue = 0;
  sleepList = 0;
  running = getproc();
  running->status = READY;

  printList(freeList);
  //printQ(readyQueue);
  //kprintf("running = %d\n", running->pid);

  printf("building pgdirs at 6MB\n");
  // create pgdir's for ALL PROCs at 6MB; Mtable at 0x4000 by hardcode in ts.s
  MTABLE = (int *)0x4000;     // Mtable at 0x4000
  mtable = (int *)0x600000;   // mtables begin at 6MB

  // Each pgdir MUST be at a 16K boundary ==>
  // 1MB at 6M has space for 64 pgdirs for 64 PROCs
  for (i=0; i<64; i++){       // for 64 PROC mtables
    for (j=0; j<2048; j++){
       mtable[j] = MTABLE[j]; // copy low 2048 entries of mtable
       // in mtable: sectionDesc=0x41E:AP=01, domain=0; CB01=1101
    }
    mtable += 4096;           // advance mtable to next 16KB
  }
}

int scheduler()
{
  char line[8];
  int pid; PROC *old=running;
  char *cp;
  kprintf("proc %d in scheduler\n", running->pid);
  if (running->status==READY)
     enqueue(&readyQueue, running);
  printQ(readyQueue);
  running = dequeue(&readyQueue);

  kprintf("next running = %d\n", running->pid);
  color = running->pid;
  
  // must switch to new running's pgdir; possibly need also flush TLB
  if (running != old){
    printf("switch to proc %d pgdir at %x ", running->pid, running->pgdir);
    printf("pgdir[2048] = %x\n", running->pgdir[2048]);
    switchPgdir((u32)running->pgdir);
  }
}  

int kgetpid()
{
  //kprintf("kgetpid: pid = %d\n", running->pid);
  return running->pid;
}

int kgetppid()
{
  //kprintf("kgetppid: pppid = %d\n", running->ppid);
  return running->ppid;
}
char *pstatus[]={"FREE   ","READY  ","SLEEP  ","BLOCK  ","ZOMBIE", " RUN  "};
int kps()
{
  int i; PROC *p; 
  for (i=0; i<NPROC; i++){
     p = &proc[i];
     kprintf("proc[%d]: pid=%d ppid=%d", i, p->pid, p->ppid);
     if (p==running)
       printf("%s ", pstatus[5]);
     else
       printf("%s", pstatus[p->status]);
     printf("name=%s\n", p->name);
  }
}

int kchname(char *s)
{ 
  kprintf("kchname: name=%s\n", s);
  strcpy(running->name, s);
  return 123;
}

int kkfork()
{
  int pid = kfork("/bin/u1");
  return pid;
}

int kkwait(int *status)
{
    int pid, e; 
    pid = kwait(&e);
    printf("write %x to status at %x in Umode\n", e, status);
    *status = e;
    return pid;
}

int ktswitch()
{
  tswitch();
}

int kgetPA()
{
  return running->pgdir[2048] & 0xFFFF0000;
}
