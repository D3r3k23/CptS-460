#include "ucode.c"

int login(const char* dev); // Returns pid

int main(int argc, char* argv[])
{
    int stdin = open("/dev/tty0", O_RDONLY);
    int stdout = open("/dev/tty0", O_WRONLY);

    int console = login("/dev/tty0");
    int S0 = login("/dev/ttyS0");
    int S1 = login("/dev/ttyS1");

    printf("init running\n");

    while (1) {
        int status;
        int pid = wait(&status);

        if (pid == console) console = login("/dev/tty0");
        else if (pid == S0)     S0 = login("/dev/ttyS0");
        else if (pid == S1)    S1 = login("/dev/ttyS1");
        else {
            // printf("init: buried an ORPHAN child P%d, status=%d\n", pid, status);
        }
    }
}

int login(const char* dev)
{
    int pid = fork();
    if (pid) { // Parent
        return pid;
    }
    else { // Child
        char cmd[32] = "login";
        strjoin(cmd, " ", dev);
        exec(cmd);
    }
}
