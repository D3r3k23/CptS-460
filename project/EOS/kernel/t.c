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

char *tab = "0123456789ABCDEF";
int BASE;
int color;

volatile int hasP1;
extern volatile int swflag;
extern KBD kbd;

//volatile int usersp, userpc;

void copy_vectors(void) {
    extern u32 vectors_start;
    extern u32 vectors_end;
    u32 *vectors_src = &vectors_start;
    u32 *vectors_dst = (u32 *)0;
    while(vectors_src < &vectors_end)
       *vectors_dst++ = *vectors_src++;
}

int mkptable()
{
  int i;
  int *pgdir, pentry;
  pentry = 0 | 0x412;
  pgdir = (int *)0x4000;

  for (i=0; i<4096; i++)
    pgdir[i] = 0;
  
  for (i=0; i<258; i++){
    pgdir[i] = pentry;
    pentry += 0x100000;
  }
}
  

int kprintf(char *fmt, ...);

extern int irq_stack_top;
extern PROC *running;

int show(int usp, int upc, int spsr)
{
  printf("SHOW pid=%d usp=%x  upc=%x spsr=%x\n", running->pid, usp, upc, spsr);
}

int copystack() // copy IRQ mode stack to SVC mode stack
{
  int *ip, *iq, i;
  //printf("copystack: running=%d\n", running->pid);
  
  ip = (int *)&(running->kstack[SSIZE]) - 1;
  iq = (int *)&irq_stack_top - 1;
  
  for (i=0; i<14; i++){ // copy 14 HIGH entries fro IRQ stack to SVC stack
      *ip = *iq;
      // printf("%x ", *ip);
      ip--; iq--;
  }
  //printf("\n");
  /*
  for (i=1; i<=14; i++){ // copy 14 HIGH entries fro IRQ stack to SVC stack
     printf("%x ", running->kstack[SSIZE-i]);
  }
  printf("\n");
  */
  // printf("%d usp=%x upc=%x\n", running->pid, running->usp, running->upc);
  running->ksp = (int *)&running->kstack[SSIZE-14];
}
  

void timer0_handler();

// IRQ interrupts handler entry point
// void __attribute__((interrupt)) kc_handler()
// KCW: use non-vectored interrupts, MUST poll the status registers to determine
//      the interrupt source
// TRY: use vectored interrupts of PL190

extern void timer0_handler();
extern void uart0_handler();
extern void uart1_handler();
extern void kbd_handler();
extern int sdc_handler();

int v31_handler()
{
  int sicstatus = *(int *)(SIC_BASE_ADDR+0);
  //printf("v31_handler: sicstatus=%x ", sicstatus);
  
  if (sicstatus & (1<<3)){
    //printf("KBD interrupt\n");
    kbd_handler();
  }
  if (sicstatus & (1<<22)){
    //printf("SDC interrupt\n"); 
    sdc_handler();
  }
} 

int vectorInt_init()
{
  printf("vectorInterrupt_init()\n");
  //  printf("t=%x u0=%x u1=%x kbd=%x\n", 
  //	 timer0_handler, uart0_handler, uart1_handler, kbd_handler);
 
  // set up vectored interrupts for (REF: KCW's armHowtoVectorIntPlan file)
  // timer0 at IRQ4, UART0 at IRQ12, UART1 at IRQ13, KBD to IRQ31:

  // (1) write to vectoraddr0 (0x100) with ISR of timer0,
  //              vectoraddr1 (0x104) with ISR of UART0,
  //              vectoraddr2 (0x108) with ISR of UART1,
  //              vectoraddr3 (0x10C) with ISR of KBD
  // all are offsets from VIC base at 0x10140000; (SIC is NOT used at all)
  *((int *)(VIC_BASE_ADDR+0x100)) = timer0_handler; 
  *((int *)(VIC_BASE_ADDR+0x104)) = uart0_handler; 
  *((int *)(VIC_BASE_ADDR+0x108)) = uart1_handler; 
  *((int *)(VIC_BASE_ADDR+0x10C)) = (int)v31_handler; 
  *((int *)(VIC_BASE_ADDR+0x110)) = (int)sdc_handler;

  
  //(2). write to intControlRegs = E=1|IRQ# =  1xxxxx
  *((int *)(VIC_BASE_ADDR+0x200)) = 0x24;    //100100 at IRQ 4
  *((int *)(VIC_BASE_ADDR+0x204)) = 0x2C;    //101100 at IRQ 12
  *((int *)(VIC_BASE_ADDR+0x208)) = 0x2D;    //101101 at IRQ 13
  *((int *)(VIC_BASE_ADDR+0x20C)) = 0x3F;    //111111 at IRQ 31
  *((int *)(VIC_BASE_ADDR+0x210)) = 0x36;    //110110 at IRQ 22

  //write 32-bit 0's to IntSelectReg to generate IRQ interrupts (1 for FIQs)
  *((int *)(VIC_BASE_ADDR+0x0C)) = 0;
}

// Then must rewrite irq_handler() to use vectors: HOW:
// IRQ => still come to irq_handler() => no need to read status registers to
// determine the interrupt source ==> should get the ISR address directly by
// reading vectoraddrReg at 0x30 => get ISR address, then bl to it.
// upon return, must write to vectoraddrReg (any value) as EOF

extern void myhandler();

void irq_chandler(int usp, int upc, int spsr)
{
  /*************** no need to read status regs as in polling ***********
  int vicstatus, sicstatus;
  vicstatus = VIC_STATUS;
  sicstatus = SIC_STATUS;  
  printf("t0=%x u0=%x kbd=%x\n", timer0_handler, uart0_handler, kbd_handler);  
  ***********************************************************************/
  running->inkmode++;
    
  int (*f)();                         // f is a function pointer
  f = *((int *)(VIC_BASE_ADDR+0x30)); // read ISR address in vectorAddr reg.
  f();                                // call the ISR function
  *((int *)(VIC_BASE_ADDR+0x30)) = 1; // write to VIC vectorAddr reg

  running->inkmode--;
  /***********
  if (f != timer0_handler && running->pid){
    printf("CH: %d: usp=%x upc=%x spsr=%x kmode=%d\n",
	   running->pid, usp, upc, spsr, running->inkmode);
  }  
  ************/
  if (swflag){
    //printf("%d IRQ switch\n", running->pid);
    copystack();
    running->ksp = &running->kstack[SSIZE - 14];
    // KCW: may call either irq_tswitch() or kcswtich() in ts.s
    irq_tswitch(usp, upc, spsr);
    //kcswitch();
  }
}

int main()
{ 
   color = RED;
   hasP1 = 0;
   KBD *kp = &kbd;
   
   fbuf_init();
   kprintf("                     Welcome to WANIX in Arm\n");
   kprintf("LCD display initialized : fbuf = %x\n", fb);
   color = CYAN;
   kbd_init();  

   // before LCD init, can't print anything yet
   vectorInt_init();
 
   /* enable UART0 IRQ */
   VIC_INTENABLE |= (1<<4);  // timer0,1 at 4 
   VIC_INTENABLE |= (1<<12); // UART0 at 12
   VIC_INTENABLE |= (1<<13); // UART1 at 13
   VIC_INTENABLE = 1<<31;    // SIC to VIC's IRQ31

   /* enable UART0 RXIM interrupt */
   UART0_IMSC = 1<<4;
      
   /* enable UART1 RXIM interrupt */
   UART1_IMSC = 1<<4;
  
   /* enable KBD IRQ */
   SIC_ENSET = 1<<3;  // KBD int=3 on SIC
   SIC_PICENSET = 1<<3;  // KBD int=3 on SIC
   // kbd->control = 1<<4;
   *(kp->base) = (1<<4);
   
   /* enable KBD IRQ */
   SIC_INTENABLE |= (1<<3); // KBD int=bit3 on SIC
   SIC_INTENABLE |= (1<<22); //SDC int=bit22 on SIC
 
   SIC_ENSET |= 1<<3;  // KBD int=3 on SIC
   SIC_ENSET |= 1<<22;  // SDC int=22 on SIC
 
   timer_init();
   timer_start(0);
   sdc_init();

   uart_init();

   init();
   kfork("init");
   hasP1 = 1;

   kprintf("P0 switch to P1, enter a key : ");
   mgetc(); printf("\n");

   while(1){
     unlock();
     while(readyQueue==0);
     tswitch();  // switch to run P1 ==> never return again
   }
}
