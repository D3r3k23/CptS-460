#include "ucode.c"

int main(int argc, char* argv[])
{
    const char* user = argv[1];

    while (1) {
        char cwd[128];
        getcwd(cwd);

        printf("%s:%s$ ", user, cwd);

        char line[128];
        gets(line);

        if (streq(line, "logout")) {
            printf("Goodbye\n");
            return 0;
        } else if (streq(line, "cd")) {
            chdir("/");
        } else {
            int pid = fork();
            if (pid) {
                int status;
                pid = wait(&status);
            } else {
                exec(line);
            }
        }
    }

    return 0;
}
