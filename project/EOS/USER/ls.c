#include "ucode.c"
#include "dir.c"

#define TOKEN_LEN 128
#include "tokenize.c"

void ls_file(const char* filename);
void ls_dir(const char* dirname);

int get_username(u16 uid, char* username);

static const char* PASSWD_FILE = "/etc/passwd";

int main(int argc, char* argv[])
{
    const char* filename = (argc >= 2) ? argv[1] : "";

    if (strlen(filename) <= 0) {
        ls_dir(".");
    } else {
        STAT st;
        int r = stat(filename, &st);
        if (r < 0) {
            printf("ls error: could not stat %s\n", filename);
        }

        if (S_ISREG(st.st_mode) || S_ISLNK(st.st_mode)) {
            ls_file(filename);
        } else if (S_ISDIR(st.st_mode)) {
            ls_dir(filename);
        } else {
            printf("Unknown file: %s\n", filename);
            return -1;
        }
    }
    return 0;
}

void ls_file(const char* filename)
{
    STAT st;
    int r = stat(filename, &st);
    u16 mode = st.st_mode;

    if (S_ISDIR(mode)) putc('d');
    if (S_ISREG(mode)) putc('-');
    if (S_ISLNK(mode)) putc('l');

    if (mode & S_IRUSR) putc('r'); else putc('-');
    if (mode & S_IWUSR) putc('w'); else putc('-');
    if (mode & S_IXUSR) putc('x'); else putc('-');
    if (mode & S_IRGRP) putc('r'); else putc('-');
    if (mode & S_IWGRP) putc('w'); else putc('-');
    if (mode & S_IXGRP) putc('x'); else putc('-');
    if (mode & S_IROTH) putc('r'); else putc('-');
    if (mode & S_IWOTH) putc('w'); else putc('-');
    if (mode & S_IXOTH) putc('x'); else putc('-');

    // Align all this into columns

    printf("  %u", st.st_nlink);
    char username[32];
    get_username(st.st_uid, username);
    printf(" %s", username);
    printf(" %d", st.st_size);

    const char* basename;
    char path_components[16][TOKEN_LEN];
    int nComponents = tokenize(filename, '/', path_components, 16);
    if (nComponents > 1) {
        basename = path_components[nComponents - 1];
    } else {
        basename = filename;
    }
    printf("    %s", basename);

    if (S_ISLNK(mode)) {
        char buf[4096];
        int r = readlink(filename, buf);
        printf(" -> %s", buf);
    }
    putc('\n');
}

void ls_dir(const char* dirname)
{
    char buf[DIR_BLKSIZE];
    int n = 0;
    for (DIR* dir = read_dir(dirname, buf, NULL); dir; dir = read_dir(NULL, buf, dir)) {
        char name[32];
        get_dir_entry_name(dir, name);

        if (!streq(name, ".") && !streq(name, "..")) {
            char path[64];
            strcpy(path, dirname);
            strjoin(path, "/", name);
            ls_file(path);
            n++;
        }
    }
    printf("total: %d\n", n);
}

int get_username(u16 uid, char* username)
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
            char fields[4][TOKEN_LEN];
            int nFields = tokenize(line, ':', fields, 4);
            if (nFields != 4) {
                printf("Error reading %s\n", PASSWD_FILE);
                return -1;
            } else {
                if (atoi(fields[3]) == uid) {
                    strcpy(username, fields[0]);
                    return 1;
                }
            }
        }
    }
    return 0;
}
