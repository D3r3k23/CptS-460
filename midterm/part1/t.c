#include "type.h"
#include "string.c"

PROC proc[NPROC];      // NPROC PROCs
PROC *freeList;        // freeList of PROCs
PROC *readyQueue;      // priority queue of READY procs
PROC *running;         // current running proc pointer

PROC *sleepList;       // list of SLEEP procs
int procsize = sizeof(PROC);

#define printf kprintf
#define gets kgets

#include "kbd.c"
#include "vid.c"
#include "exceptions.c"

#include "queue.c"

#include "wait.c"      // include wait.c file
// int kwait(int* status);
// int kexit(int exitValue);
// int ksleep(int event);
// int kwakeup(int event);

/*******************************************************
  kfork() creates a child process; returns child pid.
  When scheduled to run, child PROC resumes to body();
********************************************************/

int kprintf(char *fmt, ...);

void copy_vectors(void) {
    extern u32 vectors_start;
    extern u32 vectors_end;
    u32 *vectors_src = &vectors_start;
    u32 *vectors_dst = (u32 *)0;
    while(vectors_src < &vectors_end)
       *vectors_dst++ = *vectors_src++;
}

void IRQ_handler()
{
    int vicstatus, sicstatus;

    // read VIC SIV status registers to find out which interrupt
    vicstatus = VIC_STATUS;
    sicstatus = SIC_STATUS;
    if (vicstatus & (1<<31)){ // SIC interrupts=bit_31=>KBD at bit 3
      if (sicstatus & (1<<3)){
          kbd_handler();
       }
    }
}

// initialize the MT system; create P0 as initial running process
int init()
{
  int i;
  PROC *p;
  for (i=0; i<NPROC; i++){ // initialize PROCs
    p = &proc[i];
    p->pid = i;            // PID = 0 to NPROC-1
    p->status = FREE;
    p->priority = 0;
    p->next = p+1;
  }
  proc[NPROC-1].next = 0;
  freeList = &proc[0];     // all PROCs in freeList
  readyQueue = 0;          // readyQueue = empty

  sleepList = 0;           // sleepList = empty

  // create P0 as the initial running process
  p = running = dequeue(&freeList); // use proc[0]
  p->status = READY;
  p->priority = 0;
  p->ppid = 0;             // P0 is its own parent

  printList("freeList", freeList);
  printf("init complete: P0 running\n");
}

int body()   // process body function
{
  int c;
  char cmd[64];
  PROC* p;

  printf("proc %d starts from body()\n", running->pid);
  printf("ppid: %d\n", running->ppid);

  while(1){
    printf("***************************************\n");
    printf("proc %d running: parent=%d\n", running->pid,running->ppid);
    printList("readyQueue", readyQueue);
    printSleepList(sleepList);
    printf("Child list: ");
    for (p = running->child; p; p = p->sibling) {
      printf("[ %d] -> ", p->pid);
    }
    printf("NULL\n");
    menu();
    printf("enter a command : ");

    gets(cmd);

    if (strcmp(cmd, "ps")==0)
      do_ps();
    if (strcmp(cmd, "fork")==0)
      do_kfork();
    if (strcmp(cmd, "switch")==0)
      do_switch();
    if (strcmp(cmd, "exit")==0)
      do_exit();
   if (strcmp(cmd, "sleep")==0)
      do_sleep();
   if (strcmp(cmd, "wakeup")==0)
      do_wakeup();
    if (strcmp(cmd, "wait")==0)
      kwait(0);
  }
}

int menu()
{
  printf("**********************************\n");
  printf(" ps fork switch exit sleep wakeup \n");
  printf("**********************************\n");
}

char *status[ ] = {"FREE", "READY", "SLEEP", "ZOMBIE"};

int do_ps()
{
  int i;
  PROC *p;
  printf("PID  PPID  status\n");
  printf("---  ----  ------\n");
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    printf(" %d    %d    ", p->pid, p->ppid);
    if (p == running)
      printf("RUNNING\n");
    else
      printf("%s\n", status[p->status]);
  }
}

int kfork()  // kfork a child process to execute body() function
{
    int i;
    PROC* temp;

    PROC* p = dequeue(&freeList);
    if (!p) {
        kprintf("kfork failed\n");
        return -1;
    }
    p->ppid = running->pid;       // set ppid
    p->parent = running;          // set parent PROC pointer
    p->status = READY;
    p->priority = 1;
    p->child = 0;
    p->sibling = 0;

    if (!running->child) { // Insert at front of child list
        running->child = p;
    } else {
        if (running->child->priority < p->priority) { // Insert at 2nd child position (1st in sibling list)
            p->sibling = running->child;
            running->child = p;
        } else {
            for (temp = running->child; temp->sibling; temp = temp->sibling) {
                if (temp->sibling->priority < p->priority) { // Insert in middle of sibling list
                    p->sibling = temp->sibling;
                    temp->sibling = p;
                    break;
                }
            }
            if (!temp->sibling) { // Insert at end of child list
                temp->sibling = p;
            }
        }
    }

    // set kstack to resume to body
    //  HIGH    -1  -2  -3  -4  -5 -6 -7 -8 -9 -10 -11 -12 -13 -14
    //        ------------------------------------------------------
    // kstack=| lr,r12,r11,r10,r9,r8,r7,r6,r5, r4, r3, r2, r1, r0
    //        -------------------------------------------------|----
    //	                                              proc.ksp
    for (i = 1; i < 15; i++)
        p->kstack[SSIZE - i] = 0;        // zero out kstack

    p->kstack[SSIZE - 1] = (int)body;  // saved lr -> body()
    p->ksp = &(p->kstack[SSIZE - 14]); // saved ksp -> -14 entry in kstack

    enqueue(&readyQueue, p);
    return p->pid;
}

int do_kfork()
{
   int child = kfork();
   if (child < 0)
      printf("kfork failed\n");
   else{
      printf("proc %d kforked a child = %d\n", running->pid, child);
      printList("readyQueue", readyQueue);
   }
   return child;
}

int do_switch()
{
   tswitch();
}

int do_exit()
{
  int exitCode;
  printf("enter an exitCode value : ");
  exitCode = geti();
  kexit(exitCode);
}

int do_sleep()
{
  int event;
  printf("enter an event value to sleep on : ");
  event = geti();
  ksleep(event);
}

int do_wakeup()
{
  int event;
  printf("enter an event value to wakeup with : ");
  event = geti();
  kwakeup(event);
}

int main()
{
   color = WHITE;
   row = col = 0;

   fbuf_init();
   kprintf("Welcome to Wanix in ARM\n");
   kbd_init();

   /* enable VIC to route SIC interrupts */
   VIC_INTENABLE |= (1<<31); // SIC to VIC's IRQ31

   /* enable SIC to route KBD IRQ */
   SIC_ENSET |= (1<<3);      // KBD int=3 on SIC

   init();

   printQ(readyQueue);
   kfork();   // kfork P1 into readyQueue

   unlock();
   while(1){
     if (readyQueue)
        tswitch();
   }
}

/*********** scheduler *************/
int scheduler()
{

  printf("proc %d in scheduler()\n", running->pid);
  if (running->status == READY)
     enqueue(&readyQueue, running);
  printList("readyQueue", readyQueue);
  running = dequeue(&readyQueue);
  printf("next running = %d\n", running->pid);
  color = running->pid;

}


