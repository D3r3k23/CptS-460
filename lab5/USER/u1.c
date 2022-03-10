typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

#include "string.c"
#include "uio.c"
#include "ucode.c"

int main(char* s)
{
    u32 mode = get_cpsr() & 0x1F;
    printf("CPU mode=%x\n", mode);

    int pid = getpid();
    int ppid = getppid();

    printf("s=%x string=%s\n", s, s);

    while (1) {
        char line[64];

        printf("This is process %d in Umode at %x parent=%d entry=%x\n", pid, getPA(), ppid, get_entry() );
        umenu();
        printf("Input a command: ");
        ugetline(line);
        uprintf("\n");

        if (strcmp(line, "getpid") == 0) ugetpid();
        if (strcmp(line, "getppid") == 0) ugetppid();
        if (strcmp(line, "ps")       == 0) ups();
        if (strcmp(line, "chname")  == 0) uchname();
        if (strcmp(line, "switch") == 0) uswitch();

        if (strcmp(line, "kfork")  == 0) ukfork();
        if (strcmp(line, "wait")  == 0) uwait();
        if (strcmp(line, "exit") == 0) uexit();
    }
}
