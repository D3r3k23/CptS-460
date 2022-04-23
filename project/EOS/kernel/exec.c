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

#include "../type.h"

int exec(char *uline)
{
  int i, *ip;
  char *cp, kline[64]; 
  PROC *p = running;
  char file[32], filename[32];
  int *usp, *ustacktop;
  u32 BA, Btop, Busp, uLINE;
  char line[32];

  //printf("EXEC: proc %d uline=%x ", running->pid, uline);

  // line is in Umode image at p->pgdir[2048]&0xFFF00000=>can access from Kmode
  // char *uimage = (char *)(p->pgdir[2048] & 0xFFF00000);
  BA = (p->res->pgdir[2048] & 0xFFFFF000);
  Btop = BA + 0x100000;  // top of 1MB Uimage
  //printf("EXEC: proc %d Uimage at %x\n", running->pid, BA);
 
  uLINE = BA + ((int)uline - 0x80000000);
  kstrcpy(kline, (char *)uLINE);
  // NOTE: may use uline directly 

  //printf("EXEC: proc %d line = %s   ", running->pid, kline); 

  // first token of kline = filename
  cp = kline; i=0;
  while(*cp != ' ' && *cp){
    filename[i] = *cp;
    i++; cp++;
  } 
  filename[i] = 0;
  /*
  kstrcpy(file, "/bin/");
  kstrcat(file, filename);
  */
  BA = p->res->pgdir[2048] & 0xFFFFF000;
  //kprintf("load file %s to %x\n", file, BA);

  // load filename to Umode image 
  if (load(filename, p) <= 0 ){
    printf("exec loading error\n");
    return -1;
  }
  //  printf("after loading ");

  // copy kline to high end of Ustack in Umode image
  Btop = BA + 0x100000;
  Busp = Btop - 32;

  cp = (char *)Busp;
  kstrcpy(cp, kline);
  //printf("cp=%x contents=%s\n", cp, cp);

  p->usp = (int *)VA(0x100000 - 32);
  p->upc = (int *)VA();
  
  p->kstack[SSIZE-14] = (int)VA(0x100000 - 32); // R0 to Umode
  p->kstack[SSIZE-1] = (int)VA(0);              // ulr to Umode
  //printf("usp=%x contents=%s\n", p->usp, (char *)p->usp);
  strcpy(running->res->name, filename);

  //kprintf("kexec exit\n");
  return p->usp;       // return value may ovewrite saved R0 in kstack
}
