#ifndef T_H
#define T_H

#include "type.h"

extern PROC proc[NPROC]; // NPROC PROCs
extern PROC* freeList;   // freeList of PROCs
extern PROC* readyQueue; // priority queue of READY procs
extern PROC* running;    // current running proc pointer

extern PROC* sleepList;  // list of SLEEP procs
extern const int procsize;

/*******************************************************
  kfork() creates a child process; returns child pid.
  When scheduled to run, child PROC resumes to body();
********************************************************/

int main();
void copy_vectors(void);
void IRQ_handler(void);
void init(void);
void body(void);
int kfork(void);
void menu(void);
void do_ps(void);
void do_kfork(void);
void do_switch(void);
void do_exit(void);
void do_sleep(void);
void do_wakeup(void);
void do_wait(void);
void scheduler(void);

#endif // T_H
