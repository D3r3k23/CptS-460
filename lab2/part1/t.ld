/****************
qemu with -kernel file.bin ==> start to execute from 0x10000 ==>
file.bin must be loaded to 0x10000 ==> . = 0x10000
************/

ENTRY(start)
SECTIONS
{
   . = 0x10000;

   .text : { ts.o *(.text) }
   .data : { *(.data) }
   .bss :  { *(.bss ) }
   
   . = ALIGN(8);
   . = . + 0x10000; /* 0x1000 for 4KB; x10000=64kB of stack memory */

   stack_top = .;
}
