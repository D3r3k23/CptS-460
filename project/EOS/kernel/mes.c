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

MBUF mbuf[NPROC], *freeMbuflist;

SEMAPHORE mlock;

int mbuf_init()
{
  int i; MBUF *mp;

  printf("mbuf_init\n");
  for (i=0; i<NMBUF; i++){
    mp = &mbuf[i];
    mp->next = mp+1;
  }
  freeMbuflist = &mbuf[0];
  mbuf[NMBUF-1].next = 0;

  mlock.value = 1; mlock.queue = 0;
}

int enmqueue(MBUF **list, MBUF *mp)
{
  MBUF *p;
  if (*list==0){
    *list = mp;
  }
  else{
    p = *list;
    while(p->next)
      p = p->next;
    p->next = mp;
  }
  mp->next = 0;
}

MBUF *demqueue(MBUF **list)
{
  MBUF *p = *list;
  *list = p->next;
  return p;
}
      

MBUF *get_mbuf()
{ 
      MBUF *mp;
      P(&mlock);
       mp = freeMbuflist;
       if (mp)
          freeMbuflist = mp->next;
      V(&mlock);
      return mp;
}
   
int put_mbuf(MBUF *mp)
{
      mp->contents[0] = 0;
      P(&mlock);
         mp->next = freeMbuflist;
         freeMbuflist = mp;
      V(&mlock);
}

int cpfu(char *src, char *dest)
{
  char c;

  while( c = get_ubyte(src) ){
    *dest = c;
    dest++; src++;
  }
  *dest = 0;
  //  return (int)strlen(dest);
}

cp2u(char *src, char *dest)
{
  while(*src){
    put_ubyte(*src, dest);
    src++; dest++;
  }
  put_ubyte(0, dest);
}

int ksend(char *msg, int pid)
{
    MBUF *mp;
    char temp[128];
    //printf("%d in ksend: ", running->pid);

    // NOTE: in both PMTX and SMP, proc[i] has pid=i+1, so dec pid by 1 
    pid--;

    // reciver pid must be valid
    if ( (pid <= 0 || pid >= NPROC)){
           printf("sendMsg : invalid target pid %d\n", pid);
           return -1;
    }
    if ( proc[pid].status == FREE || proc[pid].status == ZOMBIE){
          printf("sendMsg : invalid target proc %d\n", pid);
          return -1;
    }
 
    cpfu(msg, temp);
    //printf("KSEND: proc %d sending %s to %d\n", running->pid, temp, pid);
 
    mp = get_mbuf();
    
    if (mp==0){
	 printf("no more mbuf\n");
         return -1;
    }
    mp->sender = running->pid;
    cpfu(msg, mp->contents);
      
    // deliver msg to recvID
    P(&proc[pid].res->mlock);
      enmqueue(&proc[pid].res->mqueue, mp);
    V(&proc[pid].res->mlock);
    V(&proc[pid].res->message);
    return 1;
}

int krecv(char *msg)
{
    MBUF *mp; int sender;
    //printf("%d in krecv\n", running->pid);
    P(&running->res->message);
    // if killed by signal, it may not have any message

    P(&running->res->mlock);
    mp = demqueue(&running->res->mqueue);
    V(&running->res->mlock);
    if (mp){ // only if it has a mesage
      //printf("KRECV: %d contents=%s\n", running->pid, mp->contents);
      cp2u(mp->contents, msg);
      sender = mp->sender;
      put_mbuf(mp);
      return sender;
    }
    return -1; // killed by signal
}
