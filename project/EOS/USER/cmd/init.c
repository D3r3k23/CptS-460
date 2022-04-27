#include "ucode.c"

int login(const char* dev); // Returns pid

const char* DEV_TTY_0 = "/dev/tty0";
const char* DEV_TTY_S0 = "/dev/ttyS0";
const char* DEV_TTY_S1 = "/dev/ttyS1";

int main(int argc, char* argv[])
{
    int stdin = open(DEV_TTY_0, O_RDONLY);
    int stdout = open(DEV_TTY_0, O_WRONLY);
    if (stdin != STDIN || stdout != STDOUT) {
        // !!!
        return 1;
    }

    int console = login(DEV_TTY_0);
    int S0 = login(DEV_TTY_S0);
    int S1 = login(DEV_TTY_S1);

    printf("init running\n");

    while (1) {
        int status;
        int pid = wait(&status);

        if (pid == console) console = login(DEV_TTY_0);
        else if (pid == S0)     S0 = login(DEV_TTY_S0);
        else if (pid == S1)     S1 = login(DEV_TTY_S1);
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
        close(STDIN);
        close(STDOUT);
        char cmd[32] = "login";
        strjoin(cmd, " ", dev);
        exec(cmd);
    }
}
