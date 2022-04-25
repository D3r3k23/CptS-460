#include "ucode.c"

int main(int argc, char* argv[])
{
    int total = 0;
    if (argc >= 2) { // Cat file
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
    } else { // Cat stdin
        char line[1024];
        int nLine = 0;
        char s[1];
        int n;
        while (n = read(0, s, 1)) {
            total += n;

            if (*s == '\r' || *s == '\n') {
                printf("\r%s\n", line);
                bzero(line, 1024);
            } else {
                write(1, s, 1);
                // if (*s == 3) {
                //     return 0;
                // }
                if (nLine >= 1024) {
                    printf("%s", line);
                    bzero(line, 1024);
                } else {
                    strcat(line, s);
                    nLine++;
                }
            }
        }
    }
    printf("\n");
    return !!total;
}
