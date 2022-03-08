typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;

#include "string.c"
#include "uio.c"
#include "ucode.c"

int main(char *s)
{
  int pid, ppid;
  char line[64];
  u32 mode,  *up;

  mode = get_cpsr();
  mode = mode & 0x1F;
  printf("CPU mode=%x\n", mode);

  pid = getpid();
  ppid = getppid();
  
  while(1){
    printf("ThIS IS PROCESS %d IN UMODE AT %x PARENT=%d ENTRY=%x\n",
	   pid, getPA(), ppid, get_entry() );
    umenu();
    printf("input a command : ");
    ugetline(line); 
    uprintf("\n"); 
 
    if (strcmp(line, "getpid")==0)
       ugetpid();
    if (strcmp(line, "getppid")==0)
       ugetppid();
    if (strcmp(line, "ps")==0)
       ups();
    if (strcmp(line, "chname")==0)
       uchname();
    if (strcmp(line, "switch")==0)
       uswitch();

    if (strcmp(line, "kfork")==0)
       ukfork();
    if (strcmp(line, "wait")==0)
       uwait();
    if (strcmp(line, "exit")==0)
       uexit();
  }
}


