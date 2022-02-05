.text
.code 32
   .global start
	
start:
  LDR sp, =stack_top   // set SVC stack pointer
  BL main
  B .
	
