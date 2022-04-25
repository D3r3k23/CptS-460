#include "ucode.c"
#include "str_append.c"

#define TOKEN_LEN 128
#include "tokenize.c"

static const char* PASSWD_FILE = "/etc/passwd";

int login(const char* username, const char* password);

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("login ERROR: no serial device provided\n");
        return 2;
    }
    const char* dev = argv[1];

    // Close stdin, stdout
    close(0);
    close(1);

    // Open on dev
    int stdin  = open(dev, O_RDONLY);
    int stdout  = open(dev, O_WRONLY);
    int stderror = open(dev, O_WRONLY);

    settty(dev);

    while (1) {
        char username[128];
        char password[128];

        printf("login:");
        gets(username);

        printf("password:");
        gets(password);

        login(username, password);
    }

    return 0;
}

int login(const char* username, const char* password)
{
    int passwd = open(PASSWD_FILE, O_RDONLY);
    if (passwd < 0) {
        printf("Error opening %s\n", PASSWD_FILE);
        return -1;
    }

    char buf[2048];
    int n = read(passwd, buf, 2048);

    close(passwd);

    char lines[64][TOKEN_LEN];
    int nLines = tokenize(buf, '\n', lines, 64);

    for (int i = 0; i < nLines; i++) {
        const char* line = lines[i];
        if (strlen(line) > 0) {
            char fields[7][TOKEN_LEN];
            int nFields = tokenize(line, ':', fields, 7);
            if (nFields != 7) {
                printf("Error reading %s\n", PASSWD_FILE);
                return -1;
            } else {
                const char* f_username = fields[0];
                if (streq(f_username, username)) {
                    const char* f_password = fields[1];
                    if (!streq(f_password, password)) {
                        printf("Incorrect password\n");
                        return -1;
                    } else {
                        printf("%s logging in...\n", username);
                        const char* f_gid =    fields[2];
                        const char* f_uid =     fields[3];
                        const char* f_fullname = fields[4];
                        const char* f_homedir =   fields[5];
                        const char* f_program =    fields[6];

                        int gid = atoi(f_gid);
                        int uid = atoi(f_uid);
                        chuid(uid, gid);

                        printf("Welcome home, %s!\n", f_fullname);
                        chdir(f_homedir);

                        char cmd[64];
                        strcpy(cmd, f_program);
                        str_append(cmd, f_username);
                        str_append(cmd, f_homedir);
                        exec(cmd);

                        return 0;
                    }
                }
            }
        }
    }
    printf("Username %s not found\n", username);
    return -1;
}
