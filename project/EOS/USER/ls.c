#include "ucode.c"
#include "dir.c"

void ls_file(const char* filename);
void ls_dir(const char* dirname);

int main(int argc, char* argv[])
{
    const char* filename = (argc >= 2) ? argv[1] : "";

    if (strlen(filename) <= 0) {
        char cwd[32];
        getcwd(cwd);
        ls_dir(cwd);
    } else {
        STAT st;
        stat(filename, &st);

        if (S_ISREG(st.st_mode) || S_ISLNK(st.st_mode)) {
            ls_file(filename);
        } else if (S_ISDIR(st.st_mode)) {
            ls_dir(filename);
        } else {
            printf("ls error: could not stat %s\n", filename);
            return -1;
        }
    }
    return 0;
}

void ls_file(const char* filename)
{

}

void ls_dir(const char* dirname)
{
    char buf[DIR_BLKSIZE];
    for (DIR* dir = read_dir(dirname, buf, NULL); dir; dir = read_dir(NULL, buf, dir)) {
        ls_file(dir->name);
    }
}
