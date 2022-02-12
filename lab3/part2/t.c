/********* type.h ************
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

u32 *VIC_BASE = 0x10140000;
#define VIC_STATUS     0x00/4
#define VIC_INTENABLE  0x10/4
#define VID_VADDR      0x30/4

u32 *SIC_BASE = 0x10003000;
#define SIC_ENSET      0x08/4
#define SIC_SOFTINTSET 0x10/4
#define SIC_PICENSET   0x20/4

void timer_handler();

#define BLUE   0
#define GREEN  1
#define RED    2
#define WHITE  3
#define CYAN   4
#define YELLOW 5
****************************/

#include "type.h"
#include "string.c"

extern int color;

#include "vid.c"
#include "kbd.c"
#include "timer.c"
#include "exceptions.c"

u32* VIC_BASE = (u32*)0x10140000;
u32* SIC_BASE = (u32*)0x10003000;

volatile int running = 0;

void copy_vectors(void)
{
    extern u32 vectors_start;
    extern u32 vectors_end;
    u32 *vectors_src = &vectors_start;
    u32 *vectors_dst = (u32 *)0;

    while(vectors_src < &vectors_end)
       *vectors_dst++ = *vectors_src++;
}

void IRQ_chandler()
{
  int vicstatus = *(VIC_BASE + VIC_STATUS);  // read VIC status
  int sicstatus = *(SIC_BASE + VIC_STATUS);  // read SIC status

  if (vicstatus & (1<<4)){                   // timer0 at bit4 of VIC
       timer_handler(0);
  }

  if (vicstatus & (1<<31)){     // VIC bit31 = SIC interrupts
     if (sicstatus & (1<<3)){   // KBD at bit3 on SIC
          kbd_handler();
     }
  }
}

int main()
{
   color = RED;
   row = col = 0;
   fbuf_init();
   kbd_init();

   *(VIC_BASE + VIC_INTENABLE)  |= (1<<4);         // VIC route timer0 at bit4
   *(VIC_BASE + VIC_INTENABLE)  |= (1<<31);        // VIC rount bit31

   /***********
    #define SIC_intenable  0x08/4  // enabled interrupts: read-only
      #define SIC_ENSET      0x08/4  // write to set SIC_intenable reg
   ****************/
   *(SIC_BASE + SIC_ENSET)      |= (1<<3);        // SIC bit3 = KBD interrupts

   kputs("test TIMER KBD interrupt-driven drivers\n");
   timer_init();
   timer_start(0);

   color = CYAN;
   kputs("in while(1) loop: enter keys from KBD\n");

   running = 1;
   while (running);
   kputs("Exiting\n");
}
