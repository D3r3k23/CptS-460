#include "ucode.c"
#include "str_append.c"

#define TOKEN_LEN 128
#include "tokenize.c"

int sh(const char* line);
int source(const char* filename);

int run_cmd(const char* cmd, const char** args, int nArgs);
int cmd_exists(const char* cmd);

static char user[32] = "";
static char HOME[32] = "";

int main(int argc, char* argv[])
{
    if (argc >= 2) strcpy(user, argv[1]);
    if (argc >= 3) strcpy(HOME, argv[2]);

    if (strlen(HOME) > 0) {
        chdir(HOME);
    }

    while (1) {
        char cwd[64];
        getcwd(cwd);
        printf("%s:%s$ ", user, cwd);

        char line[128];
        gets(line);

        sh(line);
    }
}

int sh(const char* line)
{
    char tokens[16][TOKEN_LEN];
    int nTokens = tokenize(line, ' ', tokens, 16);

    const char* cmd = tokens[0];
    const char* args[16];
    for (int i = 1; i < nTokens; i++) {
        args[i - 1] = tokens[i];
    }
    int nArgs = nTokens - 1;

    if (streq(cmd, "source")) {
        return source(args[1]);
    } else if (streq(cmd, "logout")) {
        printf("Goodbye\n");
        exit(0);
    } else if (streq(cmd, "pwd")) {
        char cwd[64];
        getcwd(cwd);
        printf("%s\n", cwd);
        return 0;
    } else if (streq(cmd, "cd")) {
        const char* dir;
        if (nArgs <= 0 || strlen(args[0]) <= 0) {
            dir = HOME;
        } else {
            dir = args[0];
        }
        return chdir(dir);
    } else if (strlen(cmd) > 0 && cmd_exists(cmd)) {
        if (streq(cmd, "init")) {
            printf("Error: invalid cmd\n");
            return -1;
        } else if (streq(cmd, "login")) {
            printf("You must logout first\n");
            return -1;
        } else {
            if (streq(cmd, "sh")) {
                args[0] = user;
                args[1] = HOME;
                nArgs = 2;
            }
            return run_cmd(cmd, args, nArgs);
        }
    } else {
        printf("Unknown cmd: %s\n", cmd);
        return -1;
    }
}

int source(const char* filename)
{
    int f = open(filename, O_RDONLY);
    if (f < 0) {
        printf("source error: could not open %s\n", filename);
        return 1;
    }

    char buf[2048];
    read(f, buf, 2048);

    close(f);

    char lines[128][TOKEN_LEN];
    int nLines = tokenize(buf, '\n', lines, 128);

    int r = 0;
    for (int i = 0; i < nLines; i++) {
        const char* line = lines[i];
        int status = sh(line);
        if (status) {
            r = status;
        }
    }
    return r;
}

int run_cmd(const char* cmd, const char** args, int nArgs)
{
    char cmd_line[128];
    strcpy(cmd_line, cmd);
    for (int i = 0; i < nArgs; i++) {
        str_append(cmd_line, args[i]);
    }

    int pid = fork();
    if (pid) {
        int status;
        pid = wait(&status);
        return status;
    } else {
        exec(cmd_line);

        // exec failed
        return -1;
    }
}

int cmd_exists(const char* cmd)
{

}
