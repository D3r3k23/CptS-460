#include "ucode.c"

void write_console(const char* s)
{
    int fd = open("/dev/tty0", O_WRONLY);
    if (fd != -1) {
        write(fd, s, strlen(s));
        close(fd);
    }
}

int main(int argc, char* argv[])
{
    // write_console("CAT\n\r");
    int total = 0;
    if (argc >= 2) { // cat file
        // write_console("cat file\n\r");
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
                // write_console("START CATTING\n\r");
                while (n = read(fd, buf, 1)) {
                    total += n;
                    if (isatty(STDOUT)) {
                        // write_console("WRITE CONSOLE\n\r");
                        prints(buf);
                    } else {
                        // write_console("WRITE:\n\r");
                        // write_console(buf);
                        write(STDOUT, buf, n);
                    }
                    bzero(buf, 2);
                }
                // write_console("FINSIH CATTING\n\r");
                close(fd);
            }
        }
    } else { // cat stdin
        // write_console("cat stdin\n\r");
        char lc = '\0';
        char c;
        while (read(STDIN, &c, 1)) {
            total++;
            if (isatty(STDOUT)) {
                // write_console("WRITE CONSOLE\n\r");
                if (c == '\r') {
                    printc('\n');
                }
                printc(c);
            } else {
                // write_console("WRITE\n\r");
                write(STDOUT, &c, 1);
            }
        }
        lc = c;
    }
    // write_console("CAT EXIT\n\r");
    return !total;
}
