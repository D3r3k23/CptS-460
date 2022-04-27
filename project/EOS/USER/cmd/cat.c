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
        char lc = '\0';
        char c;
        while (read(STDIN, &c, 1)) {
            total++;
            if (c == '\r' && lc != '\n' && lc != '\r') {
                putc('\n');
                putc('\r');
            } else {
                putc(c);
            }
        }
        lc = c;
    }
    return !total;
}
