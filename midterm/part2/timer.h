#ifndef TIMER_H
#define TIMER_H

#include "type.h"

#define CTL_ENABLE     ( 0x00000080 )
#define CTL_MODE       ( 0x00000040 )
#define CTL_INTR       ( 0x00000020 )
#define CTL_PRESCALE_1 ( 0x00000008 )
#define CTL_PRESCALE_2 ( 0x00000004 )
#define CTL_CTRLEN     ( 0x00000002 )
#define CTL_ONESHOT    ( 0x00000001 )

typedef volatile struct timer {
  u32 LOAD;     // Load Register, TimerXLoad                             0x00
  u32 VALUE;    // Current Value Register, TimerXValue, read only        0x04
  u32 CONTROL;  // Control Register, TimerXControl                       0x08
  u32 INTCLR;   // Interrupt Clear Register, TimerXIntClr, write only    0x0C
  u32 RIS;      // Raw Interrupt Status Register, TimerXRIS, read only   0x10
  u32 MIS;      // Masked Interrupt Status Register,TimerXMIS, read only 0x14
  u32 BGLOAD;   // Background Load Register, TimerXBGLoad                0x18
  u32 *base;
} TIMER;

void timer_init(void);
void timer_start(int n);
int timer_clearInterrupt(int n);
void timer_stop(int n);
void timer_handler(int n);

#endif // TIMER_H
