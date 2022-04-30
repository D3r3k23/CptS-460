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
                char buf[2];
                int n;
                while (n = read(fd, buf, 1)) {
                    total += n;
                    if (isatty(STDOUT)) {
                        prints(buf);
                    } else {
                        write(STDOUT, buf, n);
                    }
                    bzero(buf, 2);
                }
                close(fd);
            }
        }
    } else { // cat stdin
        char lc = '\0';
        char c;
        while (read(STDIN, &c, 1)) {
            total++;
            if (isatty(STDOUT)) {
                if (c == '\r') {
                    printc('\n');
                }
                printc(c);
            } else {
                write(STDOUT, &c, 1);
            }
        }
        lc = c;
    }
    return !total;
}
