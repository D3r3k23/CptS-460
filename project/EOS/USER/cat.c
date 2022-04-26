#include "ucode.c"

int main(int argc, char* argv[])
{
    int total = 0;
    if (argc >= 2) { // cat file
        const char* filename = argv[1];

        STAT st;
        int r = stat(filename, &st);
        if (r < 0) {
            printf("cat error: could not stat %s\n", filename);
            return -1;
        } else if (!S_ISREG(st.st_mode) && !S_ISLNK(st.st_mode)) {
            printf("cat error: %s is not a valid file\n", filename);
            return -1;
        } else {
            int fd = open(filename, O_RDONLY);
            if (fd == -1) {
                printf("cat error: could not open %s\n", filename);
                return -1;
            } else {
                char buf[1024];
                int n;
                while (n = read(fd, buf, 1024)) {
                    total += n;
                    for (int i = 0; i < n; i++) {
                        putc(buf[i]);
                    }
                }
                close(fd);
            }
        }
    } else { // cat stdin
        char line[1024];
        char s[1];
        int n;
        while (n = read(0, s, 1)) {
            total += n;
            if (*s == CTRL_C) {
                printf("\r%s\n\n", line);
                return 0;
            } else if (*s == '\r' || *s == '\n') {
                printf("\r%s\n", line);
                bzero(line, 1024);
            } else {
                write(1, s, 1);
                if (strlen(line) >= 1023) {
                    printf("\r%s", line);
                    bzero(line, 1024);
                } else {
                    strcat(line, s);
                }
            }
        }
    }
    return !total;
}
