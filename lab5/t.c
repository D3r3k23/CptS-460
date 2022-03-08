/********************************************************************
Copyright 2010-2017 K.C. Wang, <kwang@eecs.wsu.edu>
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

#include <stdint.h>
#include "type.h"
#include "string.c"
#define VA(x) (0x80000000 + (u32)x)

char *tab = "0123456789ABCDEF";
int BASE;
int color;

#include "uart.c"
#include "kbd.c"
#include "timer.c"
#include "vid.c"
#include "exceptions.c"
#include "queue.c"
#include "kernel.c"
#include "wait.c"
#include "fork.c"
#include "svc.c"
#include "sdc.c"
#include "load.c"

int kprintf(char* fmt, ...);

void copy_vectors(void)
{
    extern uint32_t vectors_start;
    extern uint32_t vectors_end;

    uint32_t *vectors_src = &vectors_start;
    uint32_t *vectors_dst = (uint32_t*)0;

    while (vectors_src < &vectors_end)
        *vectors_dst++ = *vectors_src++;
}

int mkPtable()
{
    u32* ut = (u32*)0x4000; // at 16KB
    u32 entry = 0 | 0x41E; //0x412;// AP=01 (Kmode R|W; Umode NO) domaian=0

    for (int i = 0; i < 4096; i++) {
        ut[i] = 0;
    }
    for (int i = 0; i < 258; i++) { // assume 256MB RAM + 2 I/O pages at 256MB
        ut[i] = entry;
        entry += 0x100000;
    }
}

void IRQ_handler()
{
    // read VIC status register to find out which interrupt
    int vicstatus = VIC_STATUS;
    int sicstatus = SIC_STATUS;
    // kprintf("vicstatus=%x sicstatus=%x\n", vicstatus, sicstatus);

    if (vicstatus & 0x0010) timer_handler(0);
    if (vicstatus & 0x1000) uart_handler(&uart[0]);
    if (vicstatus & 0x2000) uart_handler(&uart[1]);

    if (vicstatus & (1 << 31)) {
        if (sicstatus & (1 << 3)) kbd_handler();
        if (sicstatus & (1 << 22)) sdc_handler();
    }
}

int main()
{
    color = RED;
    row = 0;
    col = 0;
    BASE = 10;

    fbuf_init();
    kprintf("                     Welcome to WANIX in Arm\n");
    kprintf("LCD display initialized : fbuf = %x\n", fb);
    color = CYAN;
    kbd_init();

    /* enable UART0 IRQ */
    VIC_INTENABLE |= (1 << 4); // timer0,1 at 4
    VIC_INTENABLE |= (1 << 12); // UART0 at 12
    VIC_INTENABLE |= (1 << 13); // UART1 at 13
    VIC_INTENABLE |= (1 << 31); // SIC to VIC's IRQ31

    /* enable UART0 RXIM interrupt */
    UART0_IMSC |= (1 << 4);

    /* enable UART1 RXIM interrupt */
    UART1_IMSC |= (1 << 4);

    /* enable KBD IRQ */
    SIC_ENSET |= (1 << 3); // KBD int=3  on SIC
    SIC_ENSET |= (1 << 22); // SDC int=22 on SIC

    timer_init();
    timer_start(0);
    uart_init();
    UART* up = &uart[0];
    ufprintf(up, "test UART\n");

    sdc_init();

    kernel_init();

    kfork("u1");

    color = WHITE;
    kprintf("P0 switch to P1\n");

    while (1) {
        unlock();
        if (readyQueue)
            tswitch();
    }
}
