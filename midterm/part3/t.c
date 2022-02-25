#define NULL 0

#include "type.h"
#include "string.c"

void kprintf(char *fmt, ...);
#define printf kprintf

#include "queue.c"
#include "kbd.c"
#include "vid.c"

#include "pipe.c"
PIPE* kpipe;

#include "exceptions.c"
#include "wait.c"
#include "kernel.c"

void copy_vectors(void) {
    extern u32 vectors_start;
    extern u32 vectors_end;
    u32 *vectors_src = &vectors_start;
    u32 *vectors_dst = (u32 *)0;

    while(vectors_src < &vectors_end)
       *vectors_dst++ = *vectors_src++;
}

void IRQ_handler()
{
    int vicstatus, sicstatus;

    // read VIC SIV status registers to find out which interrupt
    vicstatus = VIC_STATUS;
    sicstatus = SIC_STATUS;

    if (vicstatus & (1<<31)){ // SIC interrupts=bit_31=>KBD at bit 3
      if (sicstatus & (1<<3)){
          kbd_handler();
       }
    }
}

int pipe_writer()
{
    char line[128];

    while (1) {
        kprintf("Enter a line for task%d to get : ", running->pid);
        kgets(line);

        if (strcmp(line, "exit") == 0) {
            kexit(2);
        } else {
            print_pipe(kpipe);
            printf("task%d writing line=[%s] to pipe\n", running->pid, line);
            write_pipe(kpipe, line, strlen(line) - 1);

            print_pipe(kpipe);
            printf("task%d wrote [%s] to pipe\n", running->pid,line);
        }
    }
}

int pipe_reader()
{
    char line[128];

    while (1) {
        printf("proc%d : enter number of chars to read : ", running->pid);
        int n = geti();
        if (n == 0) {
            kexit(1);
        } else {
            print_pipe(kpipe);
            printf("proc%d reading %d chars from pipe\n", running->pid, n);
            n = read_pipe(kpipe, line, n);

            print_pipe(kpipe);
            printf("proc%d read n=%d bytes from pipe : [", running->pid, n);
            for (int i = 0; i < n; i++) {
                kputc(line[i]);
            }
            printf("]\n");
        }
    }
}

int main()
{
   color = WHITE;
   row = col = 0;

   fbuf_init();
   kprintf("Welcome to Wanix in ARM\n");
   kbd_init();

   VIC_INTENABLE |= (1<<31); // SIC to VIC's IRQ31
   SIC_ENSET |= (1<<3);      // KBD int=3 on SIC

   pipe_init();
   kpipe = create_pipe();

   init();

   kprintf("P0 kfork tasks: ");

   kfork((int)pipe_writer, 1);
   kfork((int)pipe_reader, 1);

   printQ(readyQueue);

   unlock();
   while (1) {
     if (readyQueue)
        tswitch();
   }
}
