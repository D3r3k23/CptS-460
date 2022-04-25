#include "ucode.c"

int l2u(int ifd, int ofd);

int main(int argc, char* argv[])
{
    if (argc >= 3) { // l2u file
        const char* lower_filename = argv[1];
        char upper_filename[32];
        strcpy(upper_filename, argv[2]);
        STAT st;
        int r = stat(argv[2], &st);
        if (r >= 0 && S_ISDIR(st.st_mode)) {
            strjoin(upper_filename, "/", lower_filename);
        }
        r = stat(upper_filename, &st);
        if (r < 0) { // File not found
            creat(upper_filename);
        }

        int ret;
        int lower_fd = open(lower_filename, O_RDONLY);
        if (lower_fd == -1) {
            printf("l2u error: failed to open %s\n", lower_filename);
            ret = 1;
        } else {
            int upper_fd = open(upper_filename, O_WRONLY);
            if (upper_fd == -1) {
                printf("l2u error: failed to open %s\n", upper_filename);
                ret = 1;
            } else {
                ret = l2u(lower_fd, upper_fd);
                close(upper_fd);
            }
            close(lower_fd);
        }
        return ret;
    }
}

int l2u(int ifd, int ofd)
{
    int n = 0;
    char buf[1024];
    int total = 0;
    while (n = read(ifd, buf, 1024)) {
        for (int i = 0; i < n; i++) {
            char* c = buf + i;
            if (C_ISLOWER(*c)) {
                *c = to_upper(*c);
            }
        }
        write(ofd, buf, n);
        total += n;
    }
    return total;
}
