// timer.c file

/* u32 offsets = byte offset/4 */
#define TLOAD   0x00/4
#define TVALUE  0x04/4
#define TCNTL   0x08/4
#define TINTCLR 0x0C/4
#define TRIS    0x10/4
#define TMIS    0x14/4
#define TBGLOAD 0x18/4

typedef volatile struct timer{
  u32 *base;            // timer's base address; as u32 pointer
  int tick, hh, mm, ss; // per timer data area
  char clock[16]; 
}TIMER;

volatile TIMER timer[4];  

void timer_init()
{
  int i;
  TIMER *tp;

  kputs("timer_init()\n");

  for (i=0; i<4; i++){
    tp = &timer[i];
    if (i==0) tp->base = (u32 *)0x101E2000; 
    if (i==1) tp->base = (u32 *)0x101E2020; 
    if (i==2) tp->base = (u32 *)0x101E3000; 
    if (i==3) tp->base = (u32 *)0x101E3020;

    *(tp->base+TLOAD) = 0x0;        // reset
    *(tp->base+TVALUE)= 0xFFFFFFFF; // current count value
    *(tp->base+TRIS)  = 0x0;
    *(tp->base+TMIS)  = 0x0;
    *(tp->base+TCNTL) = 0x62; //011-0000=|NOTEn|Pe|IntE|-|scal=00|32-bit|0=wrap|
    *(tp->base+TBGLOAD) = 0xE0000/60;
    
    tp->tick = tp->hh = tp->mm = tp->ss = 0;
    strcpy((char *)tp->clock, "00:00:00");
  }
}

void timer_handler(int n) {
    int i;
    TIMER *t = &timer[n];

    t->tick++;

    // Use your LAB3.1 code to display a wall clock

    timer_clearInterrupt(n);  // must clear timer interrupt
} 

void timer_start(int n)       // timer_start(0), 1, etc.
{
  TIMER *tp = &timer[n];
  kputs("timer_start\n");
  *(tp->base+TCNTL) |= 0x80;  // set enable bit 7
}

int timer_clearInterrupt(int n) // timer_start(0), 1, etc.
{
  TIMER *tp = &timer[n];
  *(tp->base+TINTCLR) = 0xFFFFFFFF;
}

void timer_stop(int n) // timer_start(0), 1, etc.
{
  TIMER *tp = &timer[n];
  *(tp->base+TCNTL) &= 0x7F;  // clear enable bit 7
}
