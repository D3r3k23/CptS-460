   .global entry, main, syscall, get_usp, get_cpsr, get_entry
	.text
.code 32
// upon entry, bl main0 => r0 contains pointer to the string in ustack

entry:	
	bl main

// if main0() ever retrun:  call uexit()
        bl uexit

// user process issues int syscall(a,b,c,d) ==> a,b,c,d are in r0-r3	
syscall:
   stmfd sp!, {lr}  // save lr in Ustack
	swi #0
   ldmfd sp!, {lr}  // restore lr
   mov pc, lr

get_usp:
   mov r0, sp
   mov pc, lr

get_cpsr:
   mrs r0, cpsr
   mov pc, lr

get_entry:
   ldr r0, =entry
   mov pc, lr
	
	
