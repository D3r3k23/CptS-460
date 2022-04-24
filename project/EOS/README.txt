============= EOS.1 ===================
1. sdimage is a sdc image containing an EXT2 FS.
2. EOS kernel is in /boot/
3. all binary executables are in /bin
4. boot up EOS kernel from /boot
5. EOS kernel mounts sdimage as the root file system


============ EOS.6 ======================
Implment process scheduling by time-slice.
This requires process switch from IRQ mode (true also for dynamic prioirty)

In ARM: process switch must be performed in SVC mode.
KCW's book contains general principle, techniques and examples for
context switch in IRQ mode, but these are mostly for isolated simple cases.

To implement context switch from IRQ mode in a complete OS is rather complex.

1. Each proc has [usp,upc,ucpsr,inkmode] fields at offsets 8,12,16,20.
   Every time a proc enters SVC mode (by syscall) or IRQ mode (by interrupts)
   it increment proc's inkmode by 1. Upon entry to SVC or IRQ handler, must
   save proc's Umode [usp, upc and ucpsr]. When goUmode or exit IRQ mode, it dec
   inkmode by 1. When inkmode==0, must restore saved proc's Umode usp, upc,
   ucpsr (via SYS mode) before returning to Umode.

2. In scheduler():
   set proc's time-slice to, e.g. 8 ticks;
   clear swflag to 0

3. In timer handler:
   On each tick: dec running proc's time-slice by 1;
   When time-slice <= 0: set swflag and call irq_tswitch() from IRQ mode by

   ===================================================================
   copy IRQ stack to per proc SVC stack;
   flatten out IRQ stack
   to SVC mode and use per proc kstack to call tswitch() in SVC mode.
   preserve Umode [ups, upc, ucpsr] in proc's SVC stack.
            (give up CPU in tswitch())
   When resume in SVC mode:
        restore Umode [usp, upc] via SYS mode; set spsr to ucpsr
	ldmfd sp!, {r0-r12, pc}^
   ===================================================================
   
4. To test time-sliced proc scheduling:
   login to consle, UART0, UART1
   Run the loop command from different terminals, e.g.

   consle: loop
   UART0 : loop
   UART1 : loop

   The processes will switch when their time-slice expire.
   
5. The same technique can be used to implement dynamic proc priority.
   Due to the simple operating environment of the OS, this is NOT done yet.
   
6. loading Umode images in ELF file format is too complex for students.
   loading BINARY executable files is much simpler and easier to understand.
   eos.1.load, eos.6.load use BNIARY executable Umode image files.

