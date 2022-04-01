#include "ucode.c"

int main()
{
    int sh = fork();
    if (sh) { // P1
        while (1) {
            int status;
            int pid = wait(&status);
            if (pid == sh) { // sh process exited
                sh = fork();
                if (!sh) {
                    exec("sh");
                }
            } else {
                printf("P1 buried orphan: P%d\n", pid);
            }
        }
    } else { // P1 child (sh process)
        exec("sh");
    }
}
