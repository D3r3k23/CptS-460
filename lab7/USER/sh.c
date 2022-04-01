#include "ucode.c"

int main(int argc, char* argv[])
{
    while (1) {
        printf("[ u1 | u2 | exit ]\n");
        printf("|> ");

        char line[64];
        ugetline(line);
        printf("%c", '\n');

        char cmd[32];
        char* cp = cmd;
        char* lp = line;
        while (*lp != ' ' && *lp != '\n' && *lp != '\r' && cp < cmd + 31) {
            *cp++ = *lp++;
        }
        *cp = '\0';

        if (streq(cmd, "exit")) {
            uexit();
        } else if (streq(cmd, "u1") || streq(cmd, "u2" || streq(cmd, "u3") || streq(cmd, "u4"))) {
            int pid = ufork();
            if (pid) {
                int status;
                pid = uwait(&status);
            } else {
                exec(line);
            }
        } else {
            printf("Unknown command: %s\n", line);
        }
    }
}
