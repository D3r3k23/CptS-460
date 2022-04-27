#include "ucode.c"

int cp(int ifd, int ofd);

int main(int argc, char* argv[])
{
    if (argc < 3) {
        printf("usage: cp src dest\n");
        return 2;
    } else {
        const char* src_filename = argv[1];
        char dest_filename[32];
        strcpy(dest_filename, argv[2]);
        STAT st;
        int r = stat(argv[2], &st);
        if (r >= 0 && S_ISDIR(st.st_mode)) {
            strjoin(dest_filename, "/", src_filename);
        }
        r = stat(dest_filename, &st);
        if (r < 0) { // File not found
            creat(dest_filename);
        }

        int ret;
        int src_fd = open(src_filename, O_RDONLY);
        if (src_fd == -1) {
            printf("cp error: failed to open %s\n", src_filename);
            ret = 1;
        } else {
            int dest_fd = open(dest_filename, O_WRONLY);
            if (dest_fd == -1) {
                printf("cp error: failed to open %s\n");
                ret = 1;
            } else {
                ret = cp(src_fd, dest_fd);
                close(dest_fd);
            }
            close(src_fd);
        }
        return ret;
    }
}

int cp(int ifd, int ofd)
{
    int n;
    char buf[1024];
    int total = 0;
    while (n = read(ifd, buf, 1024)) {
        write(ofd, buf, n);
        total += n;
    }
    return total;
}
