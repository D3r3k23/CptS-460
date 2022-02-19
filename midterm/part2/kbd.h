#ifndef KBD_H
#define KBD_H

#include "type.h"

#define LSHIFT 0x12
#define RSHIFT 0x59
#define ENTER  0x5A
#define LCTRL  0x14
#define RCTRL  0xE014

#define KCNTL 0x00
#define KSTAT 0x04
#define KDATA 0x08
#define KCLK  0x0C
#define KISTA 0x10

typedef volatile struct kbd {
  char* base;
  char buf[128];
  int head, tail;
  int data, room;
} KBD;

void kbd_init(void);
void kbd_handler(void);

char kgetc(void);
int kgets(char* s);

#endif // KBD_H
