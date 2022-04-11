#include "../type.h"

int kprintf(char *fmt, ...);

int data_chandler()
{
  u32 fault_status, fault_addr, domain, status;
  int spsr = get_spsr();
  int oldcolor = color;

  color = YELLOW;
  kprintf("data_abort exception in ");
  if ((spsr & 0x1F)==0x13)
    kprintf("SVC mode\n");
  if ((spsr & 0x1F)==0x10)
    kprintf("USER mode\n");

  fault_status = get_fault_status();
  fault_addr   = get_fault_addr();
  // fault_status = 7654 3210
  //                doma status
  domain = (fault_status & 0xF0) >> 4;
  status = fault_status & 0xF;
  kprintf("status  = %x: domain=%x status=%x (0x5=Trans Invalid)\n", 
           fault_status, domain, status);
  kprintf("VA addr = %x\n", fault_addr);
  color = oldcolor;

  // if in Umode: send signal 11=SIGSEGV to process
  if ((spsr & 0x1F)==0x10){
     printf("send signal 11 to %d ", running->pid);
     running->res->signal |= (1 << 11);
  }
  else{ // PANIC and hang
    printf("PANIC: data exception in Kmode\n");
    while(1);
  }
}

void abort_chandler()
{
  int spsr = get_spsr();
  kprintf("prefetch_abort exception in ");
  if ((spsr & 0x1F)==0x13)
    kprintf("SVC mode\n");
  if ((spsr & 0x1F)==0x10)
    kprintf("USER mode\n");

  // if in Umode: send signal 4=SIGILL to process
  if ((spsr & 0x1F)==0x10){
     printf("send signal 4 to %d ", running->pid);
     running->res->signal |= (1 << 4);
  }
  else{ // PANIC and hang
    printf("PANIC: prefetch_abort in Kmode\n");
    while(1);
  }
}

void undef_chandler()
{
  int spsr = get_spsr();
  kprintf("undef exception in ");
  if ((spsr & 0x1F)==0x13)
    kprintf("SVC mode\n");
  if ((spsr & 0x1F)==0x10)
    kprintf("USER mode\n");

  // if in Umode: send signal 4=SIGILL to process
  if ((spsr & 0x1F)==0x10){
     printf("send signal 4 to %d ", running->pid);
     running->res->signal |= (1 << 4);
  }
  else{ // PANIC and hang
    printf("PANIC: prefetch_abort in Kmode\n");
    while(1);
  }
}

void fiq_handler()
{
  printf("FIQ interrupt\n");
  while(1);
}
