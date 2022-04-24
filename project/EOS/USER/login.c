#include "ucode.c"

static const char* PASSWORD_FILE = "/etc/passwd";

void login(const char* username, const char* password);
char* get_password_field(char* passwd_line, int index, char* buf);

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("login ERROR: no device provided\n");
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

void login(const char* username, const char* password)
{
    int passwd = open(PASSWORD_FILE, O_RDONLY);

    char buf[2048];
    int n = read(passwd, buf, 2048);

    close(passwd);

    char line[256];
    char* lp = line;
    for (int i = 0; i < n; i++) {
        if (buf[i] != '\n') {
            *lp++ = buf[i];
        } else {
            *lp = '\0';
            char f_username[32]; get_password_field(line, 0, f_username);
            if (streq(f_username, username)) {
                char f_password[32]; get_password_field(line, 1, f_password);
                if (streq(f_password, password)) {
                    printf("%s logging in...\n", username);
                    char f_gid[32];    get_password_field(line, 2, f_gid);
                    char f_uid[32];     get_password_field(line, 3, f_uid);
                    char f_fullname[32]; get_password_field(line, 4, f_fullname);
                    char f_homedir[32];   get_password_field(line, 5, f_homedir);
                    char f_program[32];    get_password_field(line, 6, f_program);

                    int gid = atoi(f_gid);
                    int uid = atoi(f_uid);
                    chuid(uid, gid);

                    printf("Welcome home, %s!\n", f_fullname);
                    chdir(f_homedir);

                    char cmd[64];
                    strcpy(cmd, f_program);
                    strcat(cmd, " ");
                    strcat(cmd, f_username);
                    exec(cmd);
                } else {
                    printf("Incorrect password\n");
                    return;
                }
            } else {
                memset(line, 0, 128);
                lp = line;
            }
        }
    }
    printf("Username: %s not found\n", username);
}

// username:password:gid:uid:fullname:homedir:program
char* get_password_field(char* passwd_line, int index, char* buf)
{
    // Find first char in passwd_line
    char* lp = passwd_line;
    for (int i = 0; i < index; i++) {
        while (*lp++ != ':');
    }

    // Copy field to buf
    char* fp = buf;
    while (*lp != ':' && *lp != '\n') {
        *fp++ = *lp++;
    }
    *fp = '\0';
}
