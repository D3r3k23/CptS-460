
#include "type.h"

#include "string.c"
int kprintf(char *fmt, ...);
int uprintf(char *fmt, ...);
#define printf kprintf

#include "queue.c"
#include "vid.c"
#include "pv.c"
#include "kbd.c"
#include "exceptions.c"
#include "kernel.c"
#include "timer.c"
#include "message.c"

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
    vicstatus = VIC_STATUS;
    sicstatus = SIC_STATUS;  
 
    if (vicstatus & (1<<4)){   // bit4: timer0,1
       timer_handler(0);
    }
    if (vicstatus & (1<<31)){ // SIC interrupts=bit_31=>KBD at bit 3 
      if (sicstatus & (1<<3)){
          kbd_handler();
       }
    }
}
  
int sender()
{
  char line[128];

  while(1){
    printf("Enter a line for task%d to get : \n", running->pid);
    kgets(line);
    kprints("\r\n");
    printf("task%d got a line=[%s]\n", running->pid, line);
    send(line, 2);
    printf("task%d send [%s] to pid=2\n", running->pid,line);
  }
}

int receiver()
{
  char c, *cp; 
  char line[128];
  int pid;
  while(1){
    printf("proc%d try to receive\n", running->pid);
    pid = recv(line);
    printf("proc%d received: [%s] from task%d\n", running->pid, line, pid);
  }
}
  
int main()
{ 
   color = WHITE;
   row = col = 0; 

   fbuf_init();
   kprintf("Welcome to Wanix in ARM\n");
   kbd_init();

   // enable timer0, SIC on VIC, 
   VIC_INTENABLE = 0;
   VIC_INTENABLE |= (1<<4);  // timer0 at bit4 
   VIC_INTENABLE |= (1<<31); // SIC to VIC's IRQ31
   
   // enable KBD on SIC
   SIC_ENSET = (1<<3);  // KBD int=3 on SIC

   timer_init();
   timer_start(0);

   msg_init();
   init();

   kprintf("P0 kfork tasks\n");

   kfork((int)sender, 1);
   kfork((int)receiver, 1);

   printQ(readyQueue);

   while(1){
     if (readyQueue)
        tswitch();
   }
}
