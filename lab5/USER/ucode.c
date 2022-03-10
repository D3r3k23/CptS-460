int umenu()
{
    uprintf("-----------------------------------------------\n");
    uprintf("getpid getppid ps chname switch kfork wait exit\n");
    uprintf("-----------------------------------------------\n");
}

int getpid()
{
    int pid;
    pid = syscall(0, 0, 0, 0);
    return pid;
}

int getppid()
{
    return syscall(1, 0, 0, 0);
}

int ugetpid()
{
    int pid = getpid();
    uprintf("pid = %d\n", pid);
}

int ugetppid()
{
    int ppid = getppid();
    uprintf("ppid = %d\n", ppid);
}

int ups()
{
    return syscall(2, 0, 0, 0);
}

int uchname()
{
    char s[32];
    uprintf("input a name string : ");
    ugetline(s);
    printf("\n");
    return syscall(3, s, 0, 0);
}

int uswitch()
{
    return syscall(4, 0, 0, 0);
}

int ukfork()
{
    int pid;
    char filename[64];
    printf("syscall to kernel kfork, enter a filename: ");
    ugetline(filename);
    uprintf("\n");
    pid = syscall(5, filename, 0, 0);
    printf("proc %d kforked a child %d\n", getpid(), pid);
}

int uwait()
{
    int pid, status;
    printf("syscall to kernel wait: ");
    pid = syscall(6, &status, 0, 0);
    if (pid > 0)
        printf("proc %d waited a ZOMBIE child %d status=%d\n", getpid(), pid, status);
}

int uexit()
{
    char line[64];
    printf("enter an exitValue : " );
    ugets(line);
    printf("%c", '\n');
    int exitValue = atoi(line);

    printf("syscall to kernel exit with %d\n", exitValue);
    int pid = syscall(7, exitValue, 0, 0);
}

int ugetc()
{
    return syscall(90, 0, 0, 0);
}

int uputc(char c)
{
    return syscall(91, c, 0, 0);
}
int getPA()
{
    return syscall(92, 0, 0, 0);
}
