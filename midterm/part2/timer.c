#include "timer.h"

#include "vid.h"
#include "string.h"

// timer0 base=0x101E2000; timer1 base=0x101E2020
// timer2 base=0x101E3000; timer3 base=0x101E3020

static volatile TIMER* timer[4]; // 4 timers; 2 timers per unit; at 0x00 and 0x20
static char clock[16];

static int srow, scol;
static char* blanks = "  :  :  ";
static int hh, mm, ss;
static u32 tick=0;
static int oldcolor;

void timer_init(void)
{
    int i;
    kprintf("timer_init() ");

    // set timer base address
    timer[0] = (TIMER*)(0x101E2000);
    timer[1] = (TIMER*)(0x101E2020);
    timer[2] = (TIMER*)(0x101E3000);
    timer[3] = (TIMER*)(0x101E3020);

    // set control counter regs to defaults
    for (i = 0; i < 4; i++) {
        timer[i]->LOAD    = 0x0; // reset
        timer[i]->VALUE   = 0xFFFFFFFF;
        timer[i]->RIS     = 0x0;
        timer[i]->MIS     = 0x0;
        timer[i]->LOAD    = 0x100;
        timer[i]->CONTROL = 0x62;  // 011- 0000=|NOTEn|Pe|IntE|-|scal=00|32-bit|0=wrap|
        timer[i]->BGLOAD  = 0xF0000;
    }
    strcpy(clock, "00:00:00");
    hh = mm = ss = 0;
}

void timer_start(int n)
{
    kprintf("timer_start:\n");
    timer[n]->CONTROL |= 0x80; // set enable bit
}

int timer_clearInterrupt(int n)
{
    timer[n]->INTCLR = 0xFFFFFFFF;
}

void timer_stop(int n)
{
    timer[n]->CONTROL &= 0x7F; // clear enable bit
}

void timer_handler(int n)
{
    int i;
    int ris, mis, value, load, bload;
    ris   = timer[n]->RIS;
    mis   = timer[n]->MIS;
    value = timer[n]->VALUE;
    load  = timer[n]->LOAD;
    bload = timer[n]->BGLOAD;

    ss = ++tick;
    ss %= 60;
    if ((ss % 60) == 0) {
        mm++;
        if ((mm % 60) == 0) {
            mm = 0;
        hh++;
        }
    }
    oldcolor = color;
    color = GREEN;
    for (i = 0; i < 8; i++) {
        // erasechar(clock[i], 0, 70+i);
    }

    clock[7] = '0' + (ss % 10); clock[6] = '0' + (ss / 10);
    clock[4] = '0' + (mm % 10); clock[3] = '0' + (mm / 10);
    clock[1] = '0' + (hh % 10); clock[0] = '0' + (hh / 10);

    for (i = 0; i < 8; i++) {
        kpchar(clock[i], 0, 70 + i);
    }

    timer_clearInterrupt(0);
    color = oldcolor;

    kprintf("%s", clock[n]);
}
