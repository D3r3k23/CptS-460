#define NPROC 9
PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
int procsize = sizeof(PROC);
int body();

int init()
{
  int i, j; 
  PROC *p;
  kprintf("kernel_init()\n");
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->status = READY;
    p->next = p + 1;
  }
  proc[NPROC-1].next = 0;
  freeList = &proc[0];
  sleepList = 0;
  readyQueue = 0;

  running = dequeue(&freeList);
  running->priority = 0;
  running->parent = p;
  //kprintf("running = %d\n", running->pid);
}

int scheduler()
{
  //kprintf("proc %d in scheduler\n", running->pid);
  if (running->status==READY)
     enqueue(&readyQueue, running);
  //printQ(readyQueue);
  running = dequeue(&readyQueue);
  //kprintf("next running = %d\n", running->pid);
  color = RED + running->pid;
}  

PROC *kfork(int func, int priority) 
{
  int i;
  PROC *p = dequeue(&freeList);
  if (p==0){
    kprintf("kfork failed\n");
    return (PROC *)0;
  }
  p->ppid = running->pid;
  p->parent = running;

  p->status = READY;
  p->priority = priority;
  
  // set kstack to resume to func()
  // stack = lr r12 r11 r10 r9 r8 r7 r6 r5 r4 r3 r2 r1 r0
  //         1  2   3   4   5  6  7  8  9  10 11 12 13 14
  for (i=1; i<15; i++)
    p->kstack[SSIZE-i] = 0;
  p->kstack[SSIZE-1] = (int)func;  
  p->ksp = &(p->kstack[SSIZE-14]);
 
  enqueue(&readyQueue, p);
  //printQ(readyQueue);
  //kprintf("proc%d kforked a child %d\n", running->pid, p->pid); 
  return p;
}
