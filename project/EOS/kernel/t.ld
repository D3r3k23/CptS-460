/****************
qemu with -kernel file.bin ==> start to execute from 0x10000 ==>
file.bin must be loaded to 0x10000 ==> . = 0x10000
************/
/*ENTRY(vectors_start)*/
ENTRY(reset_handler)
SECTIONS
{
  . = 0x10000;
  .text : {
      ts.o
      t.o
      *(.text)
 }
 .data : { 
      ts.o(.data) 
      t.o (.data)
      *(.data)
  }
 .bss : { *(.bss) }

  . = ALIGN(8);

  . = . + 0x4000;
  irq_stack_top = .;
  . = . + 0x1000;
  fiq_stack_top = .;
  . = . + 0x1000;
  . = . + 0x1000;
  abt_stack_top = .;
  . = . + 0x1000;
  und_stack_top = .;
  . = . + 0x1000;
  pre_stack_top = .;


}

