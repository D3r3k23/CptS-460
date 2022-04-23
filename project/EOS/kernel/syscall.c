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
extern PROC *kfork();
extern PROC *tfreeList;

// KCW: kernel code can access both VA in Kmode and Umode

char get_ubyte(char *s)
{
  return *s;
}

int put_ubyte(u8 byte, char *s)
{
  *s = byte;
}

/************ syscall functions *************/
int getpid()
{
  return running->pid;
}
       
int getppid()
{
  return running->ppid;
}

int getpri()
{
  return running->priority;
}

int ksetpri(int pri)
{
  running->priority = pri;
  return 0;
}

int getuid()
{
  return running->res->uid;
}

int kchuid(int uid, int gid)   // syscall chudi entry
{
    running->res->uid = uid;   // change gid,uid
    running->res->gid = gid;
    return 0;
}
int kgetPaddress()
{
  return running->res->paddress;
}
/*
void goUmode();
*/
int get_uword(char *b)
{ 
    return *(int *)b;
}

int put_uword(int w, char *b)
{    
  *(int *)b = w;
}


int kpause(int y)
{
  running->pause = y;  /* pause y seconds */
  ksleep(PAUSE);
  return 0;
}

/***************************************************************
  kfork(segment) creates a child task and returns the child pid.
  When scheduled to run, the child task resumes to bodypid) in 
  K mode. Its U mode environment is set to segment.
****************************************************************/

int kswitch()
{
  tswitch();
}

//char buf[64];

char *hh[ ] = {"FREE   ", "READY  ", "SLEEP  ","BLOCK  ", "ZOMBIE ", "VFORKED", 0}; 

int kps()
{
   int i,j; 
   char *p, *q, buf[16];
   buf[15] = 0;
   printf("================================\n");
   printf("pid  ppid   name      status    \n");
   printf("--------------------------------\n");

   for (i=0; i<NPROC; i++){
       if (proc[i].status == FREE)
          continue;

     if (proc[i].status != FREE){
           kprinti(proc[i].pid);  kprints("    ");
           kprinti(proc[i].ppid); kprints("    ");

       kstrcpy(buf,"       ");

       p = proc[i].res->name;
       j = 0;
       while (*p){
          buf[j] = *p; j++; p++;
       }      
       if (i==0)
       	 kprints("P0     ");
       else
         kprints(buf);
       kprints("   ");

       if (running==&proc[i])
              kprints("running");
           else
              kprints(hh[proc[i].status]);
           kprints("     ");
       }
       /*
       else{
              kprints("FREE");
       }
       */
       
  
       kprintf("\n");
   }
   printf("--------------------------------\n");
   printf("running = proc %d = %s\n", running->pid, running->res->name);
   //printList(freeList);
   //printList(tfreeList);
   return 0;
}

int chname(int y)
{
  char buf[32];
  char *cp = buf;
  int count = 0; 
  char *b = (char *)y;

  printf("running=%d %x b=%x esp=%x\n", 
          running->pid, running->res->paddress,b,getesp());

  while (count < 32){
     *cp = get_ubyte(b);
     kputc(*cp);

     if (*cp == 0) break;
     cp++; b++; count++;
  }
  buf[31] = 0;

  printf("changing name of proc %d to %s\n", running->pid, buf);
  kstrcpy(running->res->name, buf); 
  printf("done\n");
}

int chuid(int uid, int gid)
{
  running->res->uid = uid;
  running->res->gid = gid;
  return 0;
}

int ksettty(char *y)
{
  char *p; char c;
  p = running->res->tty;

  while(c = get_ubyte(y)){
    *p = c;
    p++; y++;
  }
  *p = 0;
}

int kgettty(char *y)
{
  char *p;

  p = running->res->tty;
  //printf("ktty=%s ", p);
  while(*p){
    put_ubyte(*p, y);
    p++; y++;
  }
  put_ubyte(0, y);
}

int prtable()
{
  u32 *up;
  int i;
  up = (u32 *)0x93000;
  for (i=0; i<8; i++){
    printf("%x ", *up);
    up++;
  }

  printf("\n---------------\n");
  up = (u32 *)0x94000;
  printf("contents at 0x94000\n");
  for (i=0; i<1024; i++){
    printf("%x ", *up);
    up++;
    if ((i%64)==0) kgetc();
  }
  kgetc();
  printf("\n---------------\n");
}
