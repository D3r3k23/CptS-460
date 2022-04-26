#include "ucode.c"

int l2u(int ifd, int ofd);

int main(int argc, char* argv[])
{
    if (argc == 1) {
        return l2u(STDIN, STDOUT);
    } else if (argc >= 2) {
        const char* lower_filename = argv[1];
        int lower_fd = open(lower_filename, O_RDONLY);
        if (lower_fd == -1) {
            printf("l2u error: failed to open %s\n", lower_filename);
            return 1;
        } else if (argc >= 3) {
            char upper_filename[32];
            strcpy(upper_filename, argv[2]);
            STAT st;
            int r = stat(upper_filename, &st);
            if (r >= 0 && S_ISDIR(st.st_mode)) {
                strjoin(upper_filename, "/", lower_filename);
            }
            r = stat(upper_filename, &st);
            if (r < 0) { // File not found
                creat(upper_filename);
            }
            int upper_fd = open(upper_filename, O_WRONLY);
            if (upper_fd == -1) {
                printf("l2u error: failed to open %s\n", upper_filename);
                close(lower_fd);
                return 1;
            } else {
                int r = l2u(lower_fd, upper_fd);
                close(lower_fd);
                close(upper_fd);
                return r;
            }
        }
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
