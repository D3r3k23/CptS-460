#include "ucode.c"
#include "dir.c"

#define TOKEN_LEN 128
#include "tokenize.c"

#define PIPE_READER 0
#define PIPE_WRITER 1

typedef enum redirection
{
    REDIRECT_NONE = 0,
    REDIRECT_READ,
    REDIRECT_WRITE,
    REDIRECT_APPEND
} REDIRECT;

int sh(const char* line);
int source(const char* filename);
int new_sh(void);

int sh_line(const char* line);
int exec_cmd(const char* line);
int exec_pipe(const char* head, const char* tail);
int activate_pipe(int pd[2], int RW);

REDIRECT find_redirect(const char* line);
int redirect(const char* filename, REDIRECT redirection);

int cmd_exists(const char* cmd);
int is_executable(const char* filename);

int echo(const char* line);
int expand_variables(char* src);

void sigint_handler(int sig);

// Environment variables
const char* USER = "";
const char* HOME = "";
const char* PATH = "/bin";

int main(int argc, char* argv[])
{
    if (argc >= 2) USER = argv[1];
    if (argc >= 3) HOME = argv[2];
    if (argc >= 4) PATH = argv[3];

    if (strlen(HOME) > 0) {
        chdir(HOME);
    } else {
        chdir("/");
    }

    signal(SIGINT, sigint_handler); // Catch Ctrl+C

    while (1) {
        char cwd[64];
        getcwd(cwd);
        printf("%s:%s$ ", USER, cwd);

        char line[128];
        gets(line);
        if (strlen(line) > 0) {
            sh(line);
        }
    }
}

int sh(const char* line)
{
    expand_variables(line);

    char tokens[3][TOKEN_LEN];
    int nTokens = tokenize(line, ' ', tokens, 3);

    const char* cmd = tokens[0];
    const char* arg1 = (nTokens >= 2) ? tokens[1] : NULL;

    if (streq(cmd, "logout")) {
        printf("Goodbye\n");
        exit(0);
    } else if (streq(cmd, "exit")) {
        exit((arg1) ? atoi(arg1) : 0);
    } else if (streq(cmd, "sh")) {
        return new_sh();
    } else if (streq(cmd, "source")) {
        return source(arg1);
    } else {
        int pid = fork();
        if (!pid) { // Child
            int r = sh_line(line);
            if (r == -1) {
                exit(-1);
            }
        } else { // Parent
            int status;
            pid = wait(&status);
            return status;
        }
    }
}

int source(const char* filename)
{
    if (!filename || strlen(filename) <= 0) {
        printf("source error: no sh file provided\n");
        return -1;
    } else {
        int f = open(filename, O_RDONLY);
        if (f < 0) {
            printf("error: could not open %s\n", filename);
            return -1;
        } else if (!is_executable(filename)) {
            printf("permission denied: %s\n", filename);
            return -1;
        } else {
            char buf[2048];
            read(f, buf, 2048);
            close(f);

            char lines[128][TOKEN_LEN];
            int nLines = tokenize(buf, '\n', lines, 128);

            int r = 0;
            for (int i = 0; i < nLines; i++) {
                const char* line = lines[i];
                if (line[0] == '#') { // Comment
                    continue;
                }
                int status = sh(line);
                if (status) {
                    r = status;
                }
            }
            return r;
        }
    }
}

int new_sh(void)
{
    int pid = fork();
    if (!pid) { // Child
        char cmd[32] = "sh";
        strjoin(cmd, " ", USER);
        strjoin(cmd, " ", HOME);
        strjoin(cmd, " ", PATH);
        int r = exec_cmd(cmd);
        if (r = -1) {
            exit(-1);
        }
    } else { // Parent
        int status;
        pid = wait(&status);
        return status;
    }
}

int sh_line(const char* line)
{
    printf("sh_line: %s\n", line);
    char pipe_components[8][TOKEN_LEN];
    int nPipe_components = tokenize(line, '|', pipe_components, 8);
    if (nPipe_components > 1) { // Has pipe
        char head[128];
        strcpy(head, pipe_components[0]);
        for (int i = 1; i < nPipe_components - 1; i++) {
            strjoin(head, "|", pipe_components[i]);
        }
        const char* tail = pipe_components[nPipe_components - 1];
        return exec_pipe(head, tail);
    } else {
        return exec_cmd(line);
    }
}

int exec_cmd(const char* line)
{
    printf("exec_cmd: %s\n", line);

    char tokens[16][TOKEN_LEN];
    int nTokens = tokenize(line, ' ', tokens, 16);
    for (int i = 0; i < nTokens; i++) {
        printf("token[%d]=\"%s\"\n", i, tokens[i]);
    }
    const char* cmd = tokens[0];
    if (streq(cmd, "init") || streq(cmd, "login")) {
        printf("Error: invalid cmd\n");
        return -1;
    } else {
        REDIRECT redirection = find_redirect(line);
        if (redirection && nTokens >= 3) {
            const char* redirect_filename = tokens[nTokens - 1];
            redirect(redirect_filename, redirection);
        }
        const char* args[16];
        int nArgs = nTokens - 1;
        if (redirection) {
            nArgs -= 2;
        }
        for (int i = 0; i < 16; i++) {
            if (i < nArgs) {
                args[i] = tokens[i + 1];
            } else {
                args[i] = NULL;
            }
        }
        if (streq(cmd, "pwd")) {
            return pwd();
        } else if (streq(cmd, "echo")) {
            return echo(line);
        } else if (streq(cmd, "cd")) {
            return chdir((args[0]) ? args[0] : HOME);
        } else if (cmd_exists(cmd)) {
            char cmd_line[128];
            strcpy(cmd_line, cmd);
            for (int i = 0; i < nArgs; i++) {
                strjoin(cmd_line, " ", args[i]);
            }
            exec(cmd_line);
        } else {
            printf("unknown cmd: %s\n", cmd);
            return -1;
        }
    }
}

int exec_pipe(const char* head, const char* tail)
{
    printf("pipe: head=[%s] tail=[%s]\n", head, tail);

    // 1. Create pipe
    int pd[2];
    pipe(pd);

    int head_status = 0;
    printf("pipe: forking child\n");
    int pid = fork();
    if (pid) { // Parent: tail
        printf("pipe parent: activate reader for tail\n");
        activate_pipe(pd, PIPE_READER); // 2. Activate pipe reader for parent
        int status;
        printf("pipe parent: wait\n");
        pid = wait(&status); // 3. Wait for child to finish
        if (status) {
            head_status = status;
        }
    } else { // Child: head
        printf("pipe child: activate writer for head\n");
        activate_pipe(pd, PIPE_WRITER); // 4. Activate pipe writer for child
        printf("pipe child: run head\n");
        int r = sh_line(head); // 5. Run head
        if (r == -1) {
            exit(-1);
        }
    }
    // Parent
    if (head_status != -1) {
        printf("pipe child: run tail\n");
        return exec_cmd(tail); // 6. Run tail
    } else {
        return -1;
    }
}

#if 0
int activate_pipe(int pd[2], int RW)
{
    RW = !!RW;
    close(RW);
    dup2(pd[RW], RW);
}
#else
int activate_pipe(int pd[2], int RW)
{
    switch (RW) {
        case PIPE_READER:
            dup2(pd[PIPE_READER], STDIN);
            close(pd[PIPE_WRITER]);
            return 1;
        case PIPE_WRITER:
            dup2(pd[PIPE_WRITER], STDOUT);
            close(pd[PIPE_READER]);
            return 1;
        default:
            return 0;
    }
}
#endif

REDIRECT find_redirect(const char* line)
{
    char tokens[4][TOKEN_LEN];
    int nTokens = tokenize(line, ' ', tokens, 16);

    REDIRECT redirection = REDIRECT_NONE;
    const char* redirect_filename = NULL;
    if (nTokens >= 3) {
        const char* potential_redirect_token = tokens[nTokens - 2];
        if (streq(potential_redirect_token, "<")) {
            return REDIRECT_READ;
        } else if (streq(potential_redirect_token, ">")) {
            return REDIRECT_WRITE;
        } else if (streq(potential_redirect_token, ">>")) {
            return REDIRECT_APPEND;
        }
    }
    return REDIRECT_NONE;
}

int redirect(const char* filename, REDIRECT redirection)
{
    switch (redirection) {
        case REDIRECT_READ:
            close(STDIN);
            open(filename, O_RDONLY);
            return 1;
        case REDIRECT_WRITE:
            close(STDOUT);
            open(filename, O_WRONLY | O_CREAT);
            return 1;
        case REDIRECT_APPEND:
            close(STDOUT);
            open(filename, O_WRONLY | O_CREAT | O_APPEND);
            return 1;
        default:
            return 0;
    }
}

int cmd_exists(const char* cmd)
{
    char buf[DIR_BLKSIZE];
    for (DIR* dir = read_dir(PATH, buf, NULL); dir; dir = read_dir(NULL, buf, dir)) {
        char name[32];
        get_dir_entry_name(dir, name);

        if (streq(cmd, name)) {
            char path[64];
            strcpy(path, PATH);
            strjoin(path, "/", cmd);
            return is_executable(path);
        }
    }
    return 0;
}

int is_executable(const char* filename)
{
    STAT st;
    int r = stat(filename, &st);
    if (r < 0) {
        return 0;
    } else {
        int uid = getuid();
        if (uid == 0) {
            return 1;
        } else {
            if (st.st_mode & S_IXOTH) {
                return 1;
            } else {
                int owner = (st.st_uid == uid);
                if (owner && (st.st_mode & S_IXUSR)) {
                    return 1;
                } else {
                    return 0;
                }
            }
        }
    }
}

int echo(const char* line)
{
    int start = 0;
    for (int i = 0; i < strlen(line); i++) {
        if (line[i] == '"') {
            if (!start) {
                start = i + 1;
            } else {
                int end = i - 1;
                int length = end - start + 1;
                char msg[128] = "";
                if (length > 0) {
                    strncpy(msg, &line[start], length);
                }
                printf("%s\n", msg);
                return 0;
            }
        }
    }
    printf("echo syntax error: ");
    if (start) {
        printf("no closing \"\n");
    } else {
        printf("enclose msg with\"\"\n");
    }
    return -1;
}

int expand_variables(char* src)
{
    int n = 0;
    while (*src) {
        if (*src == '$') {
            const char* var_name = src + 1;
            int var_length = strlen(var_name);
            const char* value = NULL;
            if (strncmp(var_name, "USER", 4) == 0) value = USER;
            if (strncmp(var_name, "HOME", 4) == 0) value = HOME;
            if (strncmp(var_name, "PATH", 4) == 0) value - PATH;
            if (value) {
                n++;
                int value_length = strlen(value);
                int shift_amount = value_length - (var_length + 1);
                strshift(src, shift_amount);
                memcpy(src, value, value_length);
                src += value_length;
            } else {
                src += strlen(var_length);
            }
        } else {
            src++;
        }
    }
    return n;
}

void sigint_handler(int sig)
{
    if (1) {
        signal(SIGINT, sigint_handler);
        printf("^C\n");
    }
}
