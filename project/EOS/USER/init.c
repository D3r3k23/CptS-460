#include "ucode.c"

void parent(int console, int S0, int S1);

int login(const char* dev); // Returns pid

int main(int argc, char* argv[])
{
    int Fin = open("/dev/tty0", O_RDONLY);
    int Fout = open("/dev/tty0", O_WRONLY);

    int console = login("/dev/tty0");
    int S0 = login("/dev/ttyS0");
    int S1 = login("/dev/ttyS1");

    while (1) {
        printf("init: wait for ZOMBIE child\n");
        int status;
        int pid = wait(&status);

        if (pid == console) {
            console = login("/dev/tty0");
        } else if (pid == S0) {
            S0 = login("/dev/ttyS0");
        } else if (pid == S1) {
            S1 = login("/dev/ttyS1");
        } else {
            printf("init: buried an ORPHAN child P%d\n", pid);
        }
    }

    return 0;
}

int login(const char* dev)
{
    int pid = fork();
    if (pid) { // Parent
        printf("init: fork login P%d\n", pid);
        return pid;
    }
    else { // Child
        char cmd[32];
        strcpy(cmd, "login ");
        strcat(cmd, dev);
        exec(cmd);
    }
}
