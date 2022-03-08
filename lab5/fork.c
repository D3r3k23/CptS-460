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

int body(), goUmode();

int kfork(char *filename)
{
  int i; 
  char *cp, *cq;
  char *addr;
  char line[8];
  int usize1, usize;
  int *ustacktop, *usp;
  u32 BA, Btop, Busp;

  PROC *p = dequeue(&freeList);
  if (p==0){
    kprintf("kfork failed\n");
    return -1;
  }
  p->ppid = running->pid;
  p->parent = running;
  p->parent = running;
  p->status = READY;
  p->priority = 1;
  p->cpsr = (int *)0x10;

  // build Umode page table entries: 
  printf("new proc's pgdir = %x\n", running->pgdir);
  for (i=2048; i<4096; i++)   // zero out high 2048 entries
      p->pgdir[i] = 0;
  // Assume 1MB Umode area at VA=2GB => only need one 2048_th entry
  p->pgdir[2048] = (int)(0x800000 + (p->pid-1)*0x100000 | 0xC3E);
  //                                              0xC3E=|11|0|0001|1|1110|
  //                                                     AP   DOM1   CB10
  //                                                          AP=01 for checking
  //                                                  AP=11 for Umode R/W
  // kstack must contain a syscall frame FOLLOWed by a resume frame
  // ulr = VA(0)
  // -|--- syscall frame: stmfd sp!,{r0-r12,lr} -----
  // ulrc u12 u11 u10 u9 u8 u7 u6 u5 u4 u3 u2 u1 u0   // saved Umode regs
  //-------------------------------------------------
  //  1   2   3   4   5  6  7  8  9  10 11 12 13 14

  // klr=goUmode
  // -|-- tswitch frame: stmfd sp!,{r0-r12,lr}------
  // klr r12 r11 r10 r9 r8 r7 r6 r5 r4 r3 r2 r1 r0   // saved Kmode regs
  // ----------------------------------------------------
  // 15  16  17  18  19 20 21 22 23 24 25 26 27 28
  //                                           ksp->kstack[SSIZE-28] 
  
  for (i=1; i<29; i++)  // all 28 cells = 0
    p->kstack[SSIZE-i] = 0;

  p->kstack[SSIZE-15] = (int)goUmode;  // saved klr->goUmode()
  p->ksp = &(p->kstack[SSIZE-28]);     
 
  // load filename to Umode image area at 8MB+(pid-1)*1MB
  addr = (char *)(0x800000 + (p->pid-1)*0x100000);

  if (load(filename, p)==0){
    printf("load failed\n");
    p->priority = 0; p->status = FREE;
    enqueue(&freeList, p);
    return -1;
  }
  
  p->usp = (int *)VA(0x100000); // p->usp = (int *)(0x80100000);
  p->kstack[SSIZE-1] = VA(0);   // p->kstack[SSIZE-1] = (int)0x80000000;
  
  enqueue(&readyQueue, p);

  kprintf("proc %d kforked a child %d: ", running->pid, p->pid); 
  printQ(readyQueue);

  return p->pid;
}
