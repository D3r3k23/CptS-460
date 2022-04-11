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

	.text
.code 32
.global reset_handler
.global vectors_start
.global vectors_end
.global proc
.global procsize
.global tswitch, scheduler, running, goUmode, irq_tswitch, irq_exit, swflag
.global lock, unlock, int_on, int_off, get_cpsr, get_spsr
.global switchPgdir
.global irq_stack
.global svc_handler
.global toSYS, toIRQ
.global myhandler
.global fiq_stack_top
.global setulr	
.global inUmode
.global get_fault_status, get_fault_addr
.global kcswitch

/********************************************************
mode:	USER: 10000  0x10
	FIQ : 10001  0x11
	IRQ : 10010  0x12
	SVC : 10011  0x13
        ABT : 10111  0x17
	UND : 11011  0x1B
	SYS : 11111  0x1F
********************************************************/
reset_handler:
	
// set SVC stack to HIGH END of proc[0].kstack[]
  LDR r0, =proc      // r0 points to proc's
  LDR r1, =procsize  // r1 -> procsize
  LDR r2, [r1, #0]   // r2 = procsize
  ADD r0, r0, r2     // r0 -> high end of proc[0]
  MOV sp, r0
  mov r4, r0         // r4 is a copy of r0, points PROC0's kstack top

	BL mkptable
	BL copy_vectors
	
  ldr r0, Mtable
  mcr p15, 0, r0, c2, c0, 0  // set TTBase
  mcr p15, 0, r0, c8, c7, 0  // flush TLB

  // set domain: all 01=client(check permission) 11=master(no check)
  mov r0,#0x3                // 11 for MASER
  mcr p15, 0, r0, c3, c0, 0

// enable MMU later AFTER set up the rest here
  mrc p15, 0, r0, c1, c0, 0
  orr r0, r0, #0x00000001     // set bit0
  mcr p15, 0, r0, c1, c0, 0   // write to c1
  nop
  nop
  nop
  mrc p15,0, r2, c2, c0
  mov r2, r2

  adr pc, next
next:	

  MSR cpsr, #0x92
  ldr sp, =irq_stack_top

  MSR cpsr, #0x91
  ldr sp, =fiq_stack_top

  MSR cpsr, #0x97
  ldr sp, =abt_stack_top

  MSR cpsr, #0x9B
  ldr sp, =und_stack_top

  MSR cpsr, #0x13          // back to SVC mode
  MSR spsr, #0x10          // previos mode=USER
	
//	bl main
//	b .
  ldr r0, mainstart
  mov pc, r0
	
  B .  // main() never retrun, so dead loop here
	
.align 4
Mtable:	    .word 0x4000
mainstart:  .word main
	
.align 4

/************ 1. simplest IRQ handler */ 
irq_handler:           // IRQ entry point

 sub lr, lr, #4
 stmfd	sp!, {r0-r12, lr}  // save all Umode regs in IRQ stack

	mrs r2, spsr       // save spsr in r2

// to SYS mode
	mrs r9, cpsr    
	mov r10, r9        // r10 = IRQ mode cpsr
	orr r9, r9, #0x1F
	msr cpsr, r9       // in SYS mode

// if IRQ was in Umode: must save usp, upc, spsr
	mov r0, sp         // r0=usp
	mov r1, lr         // r1=upc; r2=spsr

	msr cpsr, r10       // back to IRQ mode

	bl irq_chandler   // call irq_handler(usp,upc,spsr) in C
	bl kpsig          // handle signal in IRQ mode? 

 // ^: pop regs from kstack BUT also copy spsr into cpsr ==> back to Umode
 ldmfd sp!, {r0-r12, pc}^ // ^ : pop kstack AND to previous mode


irq_tswitch:   // tswitch is IRQ mode:  irq_switch(usp, upc, spsr)
	ldr sp, =irq_stack_top // reset IRQ sp to irq_stack_top

// r0, r1, r2 = usp, upc, spsr
// to SVC mode
	mrs r3, cpsr
	bic r3, r3, #0x1F
	orr r3, r3, #0x13
	msr cpsr, r3

// in SVC mode, use SVC stack to call tswitch()
	ldr r5, =running
	ldr r6, [r5, #0]   // r6->running PROC
	ldr sp, [r6, #4]   // SVC sp = PROC.ksp

	stmfd sp!, {r0-r2} // save r0-r2 in SVC stack

	bl tswitch
	
// when resume: in SVC mode here
	ldmfd sp!, {r0-r2}  // restore r0-r2
	
	ldr r5, =running
	ldr r6, [r5, #0]    // r6->running PROC

	ldr r4, [r6, #20]   // get running->inkmode
	cmp r4, #0          // if inkmode > 0 : kmode back to kmode 
	bgt iout
	
// inkmode = 0 ==> kmode back to Umode
// to SYS mode
	mrs r9, cpsr
	mov r10, r9        // r10 = SVC mode cpsr  
	orr r9, r9, #0x1F
	msr cpsr, r9

// in SYS mode: restore usp=r0, upc=r1, spsr=r2
	mov sp, r0
	mov lr, r1
	msr spsr, r2

// back to SVC mode
	msr cpsr, r10
// bl show	

iout:	
	ldmfd sp!, {r0-r12, pc}^

irq_exit:   // exit in IRQ mode r0=signal number

	ldr sp, =irq_stack_top // reset IRQ sp to irq_stack_top
	// to SVC mode
	mrs r1, cpsr
	bic r1, r1, #0x1F
	orr r1, r1, #0x13
	msr cpsr, r1

	ldr r5, =running
	ldr r6, [r5, #0]   // r6 = running

        ldr r7, =procsize
	ldr r8, [r7, #0]   // r8=procsize
	mov sp, r6
	add sp, r8
	bl kexit


tswitch: // tswitch() in SVC mode
//       1  2  3  4  5  6  7  8  9  10  11  12  13  14
//       ---------------------------------------------
// stack=r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r14
//       ---------------------------------------------

//lock
  MRS r0, cpsr
  ORR r0, r0, #0x80   // set bit means MASK off IRQ interrupt 
  MSR cpsr, r0

  stmfd	sp!, {r0-r12, lr}

  LDR r0, =running         // r0=&running
  LDR r1, [r0, #0]         // r1->runningPROC
  str sp, [r1, #4]         // running->ksp = sp 

  bl	scheduler

  LDR r0, =running
  LDR r1, [r0, #0]         // r1->runningPROC
  lDR sp, [r1, #4]         // sp = running->ksp

// unlock
  MRS r0, cpsr
  BIC r0, r0, #0x80   // clear bit means UNMASK IRQ interrupt
  MSR cpsr, r0
	 
  ldmfd	sp!, {r0-r12, pc}  // all in Kmode
	


svc_entry: // r4=usp, r5=pc before swi in Umode
// KCW: save r0 also ==> in kfork(), must use 

     stmfd sp!, {r0-r12, lr}

// must save Umode usp in PROC
// get usp: mov CPU to SYS mode; return sp; save it in running->usp
   ldr r5, =running   // r5=&running
   ldr r6, [r5, #0]   // r6 -> PROC of running

	// running->inkmode++
	ldr r4, [r6, #20]  // running->inkmode
	add r4, r4, #1
	str r4, [r6, #20]  // running->inkmode++
	
// KCW: ARM uses r0-r3 for first 4 parameters==> can't change them
// get usp=r13 from USER mode
   mrs r7, cpsr       // r7 = SVC mode cpsr
   mov r8, r7         // save a copy in r8
   orr r7, r7, #0x1F  // r7 = SYS mode
   msr cpsr, r7       // change cpsr in SYS mode	
// now in SYS mode, r13 same as User mode sp r14=user mode lr
   mov r9, sp         // r4=user mode sp
   mov r10, lr         // r5=user mode lr
// save r4, r5 to PROC's usp and upc
   str r9, [r6, #8]   // save usp into proc.usp at offset 8
   str r10, [r6, #12]  // save return PC into proc.upc at offset 12
// change back to SVC mode
   msr cpsr, r8

// save kmode sp into proc.ksp at offest 4; IS THIS NECESSARY?
// KCW: used in fork() to copy parent's kstack to child's kstack
   str sp, [r6,#4]    // running->ksp = sp

   bl	svc_handler  

// replace saved r0 on stack with the return value r from svc_handler()
   add sp, sp, #4     // effectively pop saved r0 off stack
   stmfd sp!,{r0}     // push r from syscall function as saved r0 to Umode

goUmode:
// later: may have switched process ==>
// must restore saved usp from the NEW PROC.usp
// temporarily mov CPU to SYS mode; load running->usp to Umode sp;
// disable IRQ interrupts
      bl kpsig
      bl reschedule
	
   MRS r7, cpsr
   orr r7, r7, #0xC0  // I and F bits=1 mask out IRQ,FIQ
   MSR cpsr, r7
	
   ldr r5, =running   // r5 = &running
   ldr r6, [r5, #0]   // r6 -> PROC of running
 	
   // set previous SR to SYS mode to access user mode sp	
   mrs r2, cpsr       // r2 = SVC mode cpsr
   mov r3, r2         // save a copy in r3
   orr r2, r2, #0x1F  // r0 = SYS mode
   msr cpsr, r2       // change cpsr in SYS mode	
   
   ldr sp, [r6, #8]   // set sp in Umode = running->usp
   msr cpsr, r3       // resotre SVC mode

// KCW: this is the caller's kstack. If we switched process, the new PROC would
// return with the origianl syscaller's kstack ==> return to the wrong context
// MUST use the current PROC saved kstack?

// replace pc in kstak with p->upc

   mov r3, sp
   add r3, r3, #52    // offset = 13*4 bytes from sp
   ldr r4, [r6, #12]
   str r4, [r3]

        // running->inkmode--
	ldr r4, [r6, #20]
	sub r4, r4, #1
	str r4, [r6, #20]
	
   // ^: pop regs from kstack BUT also copy spsr into cpsr ==> back to Umode
   ldmfd sp!, {r0-r12, pc}^ // ^ : pop kstack AND to previous mode


data_handler:  // should return to the SAME instr that cuaed the data exception
  sub	lr, lr, #4 // should subract 8 instead of 4 ==> back to SAME instruction
  stmfd	sp!, {r0-r12, lr}
  bl	data_chandler  
  ldmfd	sp!, {r0-r12, pc}^

abort_handler:  // should return to the SAME instr that cuaed the data exception
  sub	lr, lr, #4 // should subract 8 instead of 4 ==> back to SAME instruction
  stmfd	sp!, {r0-r12, lr}
  bl	abort_chandler  
  ldmfd	sp!, {r0-r12, pc}^

undef_handler:  // should return to the SAME instr that cuaed the data exception
  sub	lr, lr, #4 // should subract 8 instead of 4 ==> back to SAME instruction
  stmfd	sp!, {r0-r12, lr}
  bl	undef_chandler  
  ldmfd	sp!, {r0-r12, pc}^
	
vectors_start:
  LDR PC, reset_handler_addr
  LDR PC, undef_handler_addr
  LDR PC, svc_handler_addr
  LDR PC, prefetch_abort_handler_addr
  LDR PC, data_abort_handler_addr
  B .
  LDR PC, irq_handler_addr
  LDR PC, fiq_handler_addr

reset_handler_addr:          .word reset_handler
undef_handler_addr:          .word undef_handler
svc_handler_addr:            .word svc_entry
prefetch_abort_handler_addr: .word abort_handler
data_abort_handler_addr:     .word data_handler
irq_handler_addr:            .word irq_handler
fiq_handler_addr:            .word fiq_handler

vectors_end:

switchPgdir: // switch pgdir to new PROC's pgdir; passed in r0
  // r0 contains address of PROC's pgdir address	
  mcr p15, 0, r0, c2, c0, 0  // set TTBase
  mov r1, #0
  mcr p15, 0, r1, c8, c7, 0  // flush TLB
  mcr p15, 0, r1, c7, c10, 0  // flush TLB
  mrc p15, 0, r2, c2, c0, 0

  // set domain: all 01=client(check permission) 11=master(no check)
  //mov r0, #0x3              // 11 for MASER
  mov r0, #0x1                // 01 for client
  mcr p15, 0, r0, c3, c0, 0
	
  mov pc, lr                  // return
	

toSVC:
  // in IRQ mode now ==> switch to SYS mode 

  /* get Program Status Register; this is IRQ mode SR */
  MRS r4, cpsr
  BIC r4, r4, #0x1F  // r4 = r5 = cspr's lowest 5 bits cleared to 0
  ORR r4, r4, #0x13  // OR in 0x1F=1 1111 = SYS mode
  MSR cpsr, r1       // write to cspr, so in SYS mode now
  mov pc, lr         // return

toSYS:
  // in IRQ mode now ==> switch to SYS mode 

  /* get Program Status Register; this is IRQ mode SR */
  MRS r4, cpsr
  BIC r4, r4, #0x1F  // r4 = r5 = cspr's lowest 5 bits cleared to 0
  ORR r4, r4, #0x1F  // OR in 0x1F=1 1111 = SYS mode
  MSR cpsr, r1       // write to cspr, so in SYS mode now
  mov pc, lr         // return
	
toIRQ:
  // in SYS mode now ==> switch to IRQ mode 
  /* get Program Status Register; this is SYS mode SR */
  MRS r4, cpsr
  BIC r4, r4, #0x1F  // r4 = r5 = cspr's lowest 5 bits cleared to 0
  ORR r4, r4, #0x12  // OR in 0x12=1 0010 = IRQ mode
  MSR cpsr, r1       // write to cspr, so in IRQ mode now
  mov pc, lr         // return


// KCW: to get user mode registers in kmode:
//(1). change cpsr to SYS mode, get r11 or any other (banked) regs, r13,14
//     switch cpsr back to kmode, OR seems a simpler way by
//(2). stmfd sp,{regs}^, which pushes umode regs to kstack, then pop off kstack
//
// get user mode r13 (sp) by the same technique	
setulr:	// setulr(ulr) put ulr into Umode lr register
   mrs r7, cpsr       // r7 = SVC mode cpsr
   mov r8, r7         // save a copy in r8
   orr r7, r7, #0x1F  // r7 = SYS mode
   msr cpsr, r7       // change cpsr in SYS mode	
// now in SYS mode, r13 same as User mode sp r14=user mode lr
   mov lr, r0         // ulr = r0
// change back to SVC mode
   msr cpsr, r8
   mov pc, lr

inUmode:	// test whether previoud mode was Umode
   mrs r7, spsr       // r7 = SPAR
   AND r7, r7, #0xFF
   mov r0, #1	
   cmp r7, #0x13
   bne 1f
   sub r0, r0, #1
1:	

	mov pc, lr

// lock/unlock: turn off/on IRQ interrupts
unlock: // 
  MRS r0, cpsr
  BIC r0, r0, #0x80   // clear bit means UNMASK IRQ interrupt
  MSR cpsr, r0
  mov pc, lr	

lock: // may pass parameter in r0
  MRS r0, cpsr
  ORR r0, r0, #0x80   // set bit means MASK off IRQ interrupt 
  MSR cpsr, r0
  mov pc, lr	

int_on: // int_on(SR): restore original CPSR in r0
  MSR cpsr, r0
  mov pc, lr	

int_off: // int SR = int_off()
  MRS r0, cpsr
  mov r1, r0
  ORR r1, r1, #0x80   // set bit means MASK off IRQ interrupt 
  MSR cpsr, r1
  mov pc, lr	

get_cpsr:
	mrs r0, cpsr
	mov pc, lr
get_spsr:
	mrs r0, spsr
	mov pc, lr
	
get_fault_status:	// read and return MMU reg 5
  MRC p15,0,r0,c5,c0,0    // read DFSR
  mov pc, lr	

get_fault_addr:	         // read and return MMU reg 6
  MRC p15,0,r0,c6,c0,0    // read DFSR
  mov pc, lr	


kcswitch:   // tswitch is IRQ mode:
	mrs r2, spsr  // save spsr in r2
	
	bl copystack  // transfer HIGH 14 entries from IRQ to SVC
	ldr sp, =irq_stack_top // reset IRQ sp to irq_stack_top

// r0, r1, r2 = usp, upc, spsr
// to SYS mode
	mrs r3, cpsr
	bic r3, r3, #0x1F
	orr r3, r3, #0x1F
	msr cpsr, r3

// in SYS mode
        mov r0, sp         // usp in r0
	mov r1, lr         // upc in r1
// to SVC mode
	mrs r3, cpsr
	bic r3, r3, #0x1F
	orr r3, r3, #0x13
	msr cpsr, r3
// set SVC mode stack
	ldr r5, =running
	ldr r6, [r5, #0]  // r6->running PROC
	ldr sp, [r6, #4]  // SVC sp = PROC.ksp

	stmfd sp!, {r0-r2} // save r0-r2 in SVC stack

// call tswitch in SVC mode
	bl tswitch

// when resume: in SVC mode

        ldmfd sp!, {r0-r2}  // restore r0=usp, r1=upc r2=spsr
	
// to SYS mode
	mrs r3, cpsr
	bic r3, r3, #0x1F
	orr r3, r3, #0x1F
	msr cpsr, r3

// in SYS mode
        mov sp, r0         // usp in r0
	mov lr, r1         // upc in r1
        msr spsr, r2       // spsr to Umode

// to SVC mode
	mrs r3, cpsr
	bic r3, r3, #0x1F
	orr r3, r3, #0x13
	msr cpsr, r3

        ldmfd sp!, {r0-r12, pc}^

	.end
