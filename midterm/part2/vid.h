#ifndef VID_H
#define VID_H

#include "type.h"

extern int color;
extern int row, col;

int fbuf_init();
int clrpix(int x, int y);
int setpix(int x, int y);
int dchar(unsigned char c, int x, int y);
int scroll();
int kpchar(char c, int ro, int co);
int erasechar();
int clrcursor();
int putcursor();

int kputc(char c);
int kprints(char *s);
int kprintx(int x);
int kprintu(int x);
int kprinti(int x);
int kprintf(char* fmt, ...);

#endif // VID_H
