#include "ucode.c"

int main(int argc, char* argv[])
{
    printf("fe:\n");
    for (int i = 0; i < argc; i++) {
        printf("arg[%d]=\"%s\"\n", i, argv[i]);
    }

    if (argv < 2) {
        printf("Error: No cmd provided\n");
        return 2;
    }

    char cmd_line[128];
    cmd_line[0] = '\0';

    for (int i = 1; i < argc; i++) {
        strcat(cmd_line, argv[i]);
        if ( i < argv - 1) {
            strcat(cmd_line, " ");
        }
    }

    int pid = ufe(cmd_line);
    if (pid < 0) {
        return 1;
    } else {
        int status;
        wait(&status);

        printf("Zombie child: P%d | status=%d\n", pid, status);
        return 0;
    }
}
