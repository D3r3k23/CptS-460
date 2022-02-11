.text
.code 32
   .global reset_handler

reset_handler:
  LDR sp, =stack_top   // set SVC mode stack pointer
  BL main
  B .
	
